#ifndef __COMMON_POOL_H
#define __COMMON_POOL_H

#include <list>
#include <mutex>
#include <algorithm>
#include <chrono>
#include <thread>

////////////////////////////////////////////////////////////////
//
// 连接池封装
//
////////////////////////////////////////////////////////////////

template<typename Connection>
class ConnectionPool {
public:
    //// Internal types
    struct ConnectionInfo {
        Connection *conn;
        time_t last_used;
        bool in_use;

        ConnectionInfo(Connection *c) :
                conn(c),
                last_used(time(0)),
                in_use(true) {
        }

        // Strict weak ordering for ConnectionInfo objects.
        // 
        // This ordering defines all in-use connections to be "less
        // than" those not in use.  Within each group, connections
        // less recently touched are less than those more recent.
        bool operator<(const ConnectionInfo &rhs) const {
            const ConnectionInfo &lhs = *this;
            return lhs.in_use == rhs.in_use ?
                   lhs.last_used < rhs.last_used :
                   lhs.in_use;
        }
    };

    typedef std::list<ConnectionInfo> PoolT;
    typedef typename PoolT::iterator PoolIt;

public:
    ConnectionPool() {}

    virtual ~ConnectionPool() {}

    /// Returns true if pool is empty
    bool empty() const { return pool_.empty(); }


    /// Grab a free connection from the pool.
    ///
    /// This method creates a new connection if an unused one doesn't
    /// exist, and destroys any that have remained unused for too long.
    /// If there is more than one free connection, we return the most
    /// recently used one; this allows older connections to die off over
    /// time when the caller's need for connections decreases.
    ///
    /// Do not delete the returned pointer.  This object manages the
    /// lifetime of connection objects it creates.

    virtual Connection *grab() {
        std::lock_guard<std::mutex> guard(mutex_);
        if (Connection *mru = find_mru()) {
            return mru;
        } else {
            // No free connections, so create and return a new one.
            pool_.push_back(ConnectionInfo(create()));
            return pool_.back().conn;
        }
    }


    /// Marks the connection as no longer in use.
    ///
    /// The pool updates the last-used time of a connection only on
    /// release, on the assumption that it was used just prior.  There's
    /// nothing forcing you to do it this way: your code is free to
    /// delay releasing idle connections as long as it likes.  You
    /// want to avoid this because it will make the pool perform poorly;
    /// if it doesn't know approximately how long a connection has
    /// really been idle, it can't make good judgements about when to
    /// remove it from the pool.

    virtual void release(const Connection *pc) {
        std::lock_guard<std::mutex> guard(mutex_);
        for (PoolIt it = pool_.begin(); it != pool_.end(); ++it) {
            if (it->conn == pc) {
                it->in_use = false;
                it->last_used = time(0);
                break;
            }
        }
    }


    /// Removes the given connection from the pool
    ///
    /// If you mean to simply return a connection to the pool after
    /// you're finished using it, call release() instead.  This method
    /// is primarily for error handling: you somehow have figured out
    /// that the connection is defective, so want it destroyed and
    /// removed from the pool.  If you also want a different connection
    /// to retry your operation on, call exchange() instead.
    ///
    void remove(const Connection *pc) {
        std::lock_guard<std::mutex> guard(mutex_);
        for (PoolIt it = pool_.begin(); it != pool_.end(); ++it) {
            if (it->conn == pc) {
                remove(it);
                return;
            }
        }
    }

    /// Remove all unused connections from the pool
    void shrink() { clear(false); }

    void removeAll() { clear(true); }

protected:
    /// Drains the pool, freeing all allocated memory.
    ///
    /// A derived class must call this in its dtor to avoid leaking all
    /// Connection objects still in existence.  We can't do it up at
    /// this level because this class's dtor can't call our subclass's
    /// destroy() method.
    ///
    /// all if true, remove all connections, even those in use
    void clear(bool all = true) {
        std::lock_guard<std::mutex> guard(mutex_);

        PoolIt it = pool_.begin();
        while (it != pool_.end()) {
            if (all || !it->in_use) {
                remove(it++);
            } else {
                ++it;
            }
        }
    }

    /// Create a new connection
    ///
    /// Subclasses must override this.
    ///
    /// Essentially, this method lets your code tell ConnectionPool
    /// what server to connect to, what login parameters to use, what
    /// connection options to enable, etc.  ConnectionPool can't know
    /// any of this without your help.
    ///
    /// A connected Connection object
    virtual Connection *create() = 0;

    /// Destroy a connection
    ///
    /// Subclasses must override this.
    ///
    /// This is for destroying the objects returned by create().
    /// Because we can't know what the derived class did to create the
    /// connection we can't reliably know how to destroy it.
    virtual void destroy(Connection *) = 0;

    /// Returns the current size of the internal connection pool.
    size_t size() const { return pool_.size(); }

private:
    //// Internal support functions
    Connection *find_mru() {
        PoolIt mru = std::max_element(pool_.begin(), pool_.end());
        if (mru != pool_.end() && !mru->in_use) {
            mru->in_use = true;
            return mru->conn;
        } else {
            return 0;
        }
    }

    void remove(const PoolIt &it) {
        // Don't grab the mutex.  Only called from other functions that do
        // grab it.
        destroy(it->conn);
        pool_.erase(it);
    }


    //// Internal data
    PoolT pool_;
    std::mutex mutex_;
};

////////////////////////////////////////////////////////////////
//
// 连接池封装（有上限）
//
////////////////////////////////////////////////////////////////
template<typename Connection, typename PoolType = ConnectionPool<Connection> >
class ConnectionPoolWithLimit : public PoolType {
public:
    typedef PoolType Base;

    ConnectionPoolWithLimit(const unsigned int maxconn = 1) {
        conns_max_ = maxconn;
        conns_in_use_ = 0;
        grab_waittime_ = -1;
    }

    virtual ~ConnectionPoolWithLimit() {
        Base::clear();
    }

    //
    // aquire the resource
    //
    Connection *acquire() {
        return this->grab();
    }

    //
    // release the resource
    //
    void putback(const Connection *conn) {
        if (conn)
            this->release(conn);
    }

    //
    // pool maximum size
    //
    void setMaxConn(unsigned int maxconn) {
        conns_max_ = maxconn;
    }

    //
    // -1 - waiting for resource forever(your first choice)
    // N  - waiting for resource with timeout(ms)
    // 0  - no waiting
    //
    void setGrabWaitTime(int ms) {
        grab_waittime_ = ms;
    }

    //
    // create all the resource (using this at starting point)
    // once created, will not going to release it, you can aquire it
    //
    void createAll() {
        std::vector<Connection *> conns;
        for (unsigned int i = 0; i < conns_max_; i++)
            conns.push_back(acquire());

        for (size_t i = 0; i < conns.size(); i++)
            putback(conns[i]);
    }

public:
    virtual Connection *grab() {
        // no wait
        if (grab_waittime_ == 0) {
            if (conns_in_use_ >= conns_max_) {
                std::cout << "grab failed: not engouh" << std::endl;
                return NULL;
            }
        }
            // waiting for release forever
        else if (grab_waittime_ < 0) {
            while (conns_in_use_ >= conns_max_) {
                std::chrono::milliseconds ms(1);
                std::this_thread::sleep_for(ms);
            }
        }
            // wating for release during grab_waittime_(ms)
        else if (grab_waittime_ > 0) {
            int tries = 0;
            while (conns_in_use_ >= conns_max_) {
                // waiting for release
                std::chrono::milliseconds ms(grab_waittime_);
                std::this_thread::sleep_for(ms);

                if (++tries > 1000)
                    return NULL;
            }
        }

        {
            std::lock_guard<std::mutex> guard(mutex_);
            ++conns_in_use_;
        }

        return Base::grab();
    }

    virtual void release(const Connection *pc) {
        Base::release(pc);

        {
            std::lock_guard<std::mutex> guard(mutex_);
            --conns_in_use_;
        }
    }

    virtual Connection *create() = 0;

    virtual void destroy(Connection *cp) {
        delete cp;
    }

protected:
    // Number of connections currently in use
    volatile unsigned int conns_in_use_;
    unsigned int conns_max_;
    int grab_waittime_;
    std::mutex mutex_;
};

template<typename ConnType, typename PoolType>
class ScopedConnection {
public:
    explicit ScopedConnection(PoolType *pool = &PoolType::instance())
            : pool_(pool), connection_(pool ? (ConnType *) pool->acquire() : NULL) {}

    ~ScopedConnection() {
        if (pool_) pool_->putback(connection_);
    }

    ConnType *operator->() const { return connection_; }

    ConnType &operator*() const { return *connection_; }

    operator void *() const { return connection_; }

    ConnType *get() { return connection_; }

private:
    PoolType *pool_;
    ConnType *connection_;
};


#endif // __COMMON_POOL_H