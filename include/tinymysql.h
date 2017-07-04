#ifndef TINYWORLD_TINYMYSQL_H
#define TINYWORLD_TINYMYSQL_H

#include <mysql++/mysql++.h>
#include "pool.h"
#include "pool_sharding.h"

class MySqlConnection : public mysqlpp::Connection {
public:
    MySqlConnection();

    MySqlConnection(const std::string &url);

    virtual ~MySqlConnection() {}

    //
    // TinyURL Format:
    // mysql://username:passwd@host:port/db?shard=value...
    //
    bool connectByURL(const std::string &url);

    int shard() const { return shard_; }

private:
    int shard_;
};

class MySqlConnectionPool : public ConnectionPoolWithLimit<mysqlpp::Connection, mysqlpp::ConnectionPool> {
public:
    MySqlConnectionPool();

    virtual ~MySqlConnectionPool() {}

    static MySqlConnectionPool& instance() {
        static MySqlConnectionPool mypool;
        return mypool;
    }


    //
    // TinyURL Format:
    //  mysql://username:passwd@host:port/db?shard=value&idletime=xx&maxconn=xx...
    //  shard    - shard
    //  idletime - mysql will close the idle client
    //  maxconn  - pool's biggest connection number
    void setServerAddress(const std::string &url);

    void setIdleTime(unsigned int seconds) {
        wait_timeout_ = seconds;
    }

    int shard() const { return shard_; }

    const std::string &url() const { return url_; }

    void connect(const std::string& url) {
        setServerAddress(url);
        createAll();
    }

protected:
    virtual mysqlpp::Connection *create();


    virtual unsigned int max_idle_time() {
        // Set our idle time at an example-friendly 3 seconds.  A real
        // pool would return some fraction of the server's connection
        // idle timeout instead.
        // show variables like '%timeout%';
        return wait_timeout_;
    }

private:
    // MySQL: show variables like '%timeout%';
    unsigned int wait_timeout_;

    // Our connection parameters
    std::string url_;

    int shard_;
};


class MySqlShardingPool : public ShardingConnectionPool<MySqlConnection, MySqlConnectionPool> {
public:
    typedef ShardingConnectionPool<MySqlConnection, MySqlConnectionPool> Base;

    MySqlShardingPool(int shardnum = 1) {
        Base::setShardNum(shardnum);
    }

    ~MySqlShardingPool() {
        fini();
    }

    static MySqlShardingPool *instance();

public:
    bool init();

    void fini();

    bool addShardings(const std::vector<std::string> &urls);

    bool addSharding(const std::string &url);
};

typedef ScopedConnection<MySqlConnection, MySqlConnectionPool> ScopedMySqlConnection;
typedef ScopedConnectionByShard<MySqlConnection, MySqlShardingPool> MySqlConnectionByShard;
typedef ScopedConnectionByHash<MySqlConnection, MySqlShardingPool> MySqlConnectionByHash;
typedef ScopedConnectionByKey<MySqlConnection, MySqlShardingPool> MySqlConnectionByKey;

#endif //TINYWORLD_TINYMYSQL_H
