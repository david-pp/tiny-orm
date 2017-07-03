#ifndef TINYWORLD_POOL_SHARDING_H
#define TINYWORLD_POOL_SHARDING_H

#include "sharding.h"

template<typename ConnType, typename PoolType>
class ShardingConnectionPool : public Sharding<PoolType> {
public:
    typedef Sharding<PoolType> Base;

    ShardingConnectionPool(int shardnum = 1)
            : Base(shardnum) {
    }

    virtual ~ShardingConnectionPool() {}

public:
    ConnType *acquireByShard(int shard) {
        PoolType *pool = this->getShardByID(shard);
        if (pool) {
            return (ConnType *) pool->acquire();
        }

        return NULL;
    }

    ConnType *acquireByHash(uint32_t hash) {
        PoolType *pool = this->getShardByHash(hash);
        if (pool) {
            return (ConnType *) pool->acquire();
        }

        return NULL;
    }

    ConnType *acquireByKey(const std::string &key) {
        PoolType *pool = this->getShardByKey(key);
        if (pool) {
            return (ConnType *) pool->acquire();
        }

        return NULL;
    }

    void putback(const ConnType *conn) {
        if (!conn) return;

        PoolType *pool = this->getShardByID(conn->shard());
        if (pool) {
            return pool->putback(conn);
        }
    }
};


template<typename ConnType, typename PoolType>
class ScopedConnectionByShard {
public:
    explicit ScopedConnectionByShard(int shard, PoolType *pool = PoolType::instance())
            : pool_(pool), connection_(pool->acquireByShard(shard)) {}

    ~ScopedConnectionByShard() {
        if (pool_) pool_->putback(connection_);
    }

    ConnType *operator->() const { return connection_; }

    ConnType &operator*() const { return *connection_; }

    operator void *() const { return connection_; }

private:
    PoolType *pool_;
    ConnType *connection_;
};

template<typename ConnType, typename PoolType>
class ScopedConnectionByHash {
public:
    explicit ScopedConnectionByHash(uint32_t hash, PoolType *pool = PoolType::instance())
            : pool_(pool), connection_(pool->acquireByHash(hash)) {}

    ~ScopedConnectionByHash() {
        if (pool_) pool_->putback(connection_);
    }

    ConnType *operator->() const { return connection_; }

    ConnType &operator*() const { return *connection_; }

    operator void *() const { return connection_; }

private:
    PoolType *pool_;
    ConnType *connection_;
};


template<typename ConnType, typename PoolType>
class ScopedConnectionByKey {
public:
    explicit ScopedConnectionByKey(const std::string &key, PoolType *pool = PoolType::instance())
            : pool_(pool), connection_(pool->acquireByKey(key)) {}

    ~ScopedConnectionByKey() {
        if (pool_) pool_->putback(connection_);
    }

    ConnType *operator->() const { return connection_; }

    ConnType &operator*() const { return *connection_; }

    operator void *() const { return connection_; }

private:
    PoolType *pool_;
    ConnType *connection_;
};



#endif //TINYWORLD_POOL_SHARDING_H
