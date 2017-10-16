// Copyright (c) 2017 david++
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef TINYWORLD_TINYORM_MYSQL_H
#define TINYWORLD_TINYORM_MYSQL_H

#include <vector>
#include <cstdarg>
#include <unordered_set>
#include <functional>
#include "tinyorm.h"
#include "tinymysql.h"
#include "tinylogger.h"

class TinyMySqlORM {
public:
    typedef MySqlConnectionPool PoolType;
    typedef mysqlpp::Connection ConnectionType;

    //
    // 构造: 支持两种方式:
    //   1. 连接池
    //   2. 指定连接
    //
    TinyMySqlORM(MySqlConnectionPool *pool = &MySqlConnectionPool::instance()) {
        if (pool) {
            pool_ = pool;
            mysql_ = pool->grab();
        }
    }

    TinyMySqlORM(mysqlpp::Connection *connection) {
        if (connection) {
            mysql_ = connection;
            pool_ = nullptr;
        }
    }

    //
    // 析构: 如果是用连接池初始化的则把连接放回
    //
    ~TinyMySqlORM() {
        if (pool_)
            pool_->putback(mysql_);
    }

    //
    // 更新所有表格的结构
    //
    bool showTables(std::unordered_set<std::string> &tables);

    bool updateTables();

    //
    // 创建、删除、自动更新表结构
    //
    template<typename T>
    bool createTableByType();

    bool createTableByName(const std::string &name);

    template<typename T>
    bool dropTableByType();

    bool dropTableByName(const std::string &name);

    template<typename T>
    bool updateTableByType();

    bool updateTableByName(const std::string &name);

    //
    // 针对某个对象的数据库操作
    //
    template<typename T>
    bool select(T &obj);

    template<typename T>
    bool insert(T &obj);

    template<typename T>
    bool replace(T &obj);

    template<typename T>
    bool update(T &obj);

    template<typename T>
    bool del(T &obj);

    //
    // 数据库批量加载
    //
    template<typename T>
    using Records = std::vector<std::shared_ptr<T>>;

    template<typename T>
    bool loadFromDB(Records<T> &records, const char *clause, ...);

    template<typename T, typename TSet>
    bool loadFromDB(TSet &records, const char *clause, ...);

    template<typename T, typename TMultiIndexSet>
    bool loadFromDB2MultiIndexSet(TMultiIndexSet &records, const char *clause, ...);

    template<typename T>
    bool loadFromDB(const std::function<void(std::shared_ptr<T>)> &callback, const char *clause, ...);

    template<typename T>
    bool vloadFromDB(const std::function<void(std::shared_ptr<T>)> &callback, const char *clause, va_list ap);

    //
    // 数据库批量删除
    //
    template<typename T>
    bool deleteFromDB(const char *where, ...);

public:
    //
    // Generate SQL
    //
    template<typename T>
    bool makeSelectQuery(mysqlpp::Query &query, const T &obj, TableDescriptor<T> *td = nullptr);

    template<typename T>
    bool makeInsertQuery(mysqlpp::Query &query, const T &obj, TableDescriptor<T> *td = nullptr);

    template<typename T>
    bool makeReplaceQuery(mysqlpp::Query &query, const T &obj, TableDescriptor<T> *td = nullptr);

    template<typename T>
    bool makeUpdateQuery(mysqlpp::Query &query, const T &obj, TableDescriptor<T> *td = nullptr);

    template<typename T>
    bool makeDeleteQuery(mysqlpp::Query &query, const T &obj, TableDescriptor<T> *td = nullptr);


protected:
    bool updateExistTable(TableDescriptorBase* td);

    bool createTable(TableDescriptorBase* td);

    bool dropTable(TableDescriptorBase* td);

    bool updateTable(TableDescriptorBase* td);


    //
    // value1,value2,...,valueN
    //
    template<typename T>
    void makeValueList(mysqlpp::Query &query, T &obj, TableDescriptor<T> *td, const FieldDescriptorList &fdlist);

    //
    // key1=value1,key2=valule2,...,keyN=valueN
    //
    template<typename T>
    void makeKeyValueList(mysqlpp::Query &query, T &obj, TableDescriptor<T> *td, const FieldDescriptorList &fdlist,
                          const std::string &seperator = ",");


    template<typename T>
    bool fieldToQuery(mysqlpp::Query &query, T &obj, TableDescriptor<T> *td, FieldDescriptor::Ptr fd);

    template<typename T>
    bool recordToObject(mysqlpp::Row &record, T &obj, TableDescriptor<T> *td);


private:
    mysqlpp::Connection *mysql_ = nullptr;
    MySqlConnectionPool *pool_ = nullptr;
};

#include "tinyorm_mysql.in.h"

typedef TinyMySqlORM TinyORM;

template <typename T>
using Object2DB = Object2DB_T<T, TinyMySqlORM>;

#endif //TINYWORLD_TINYORM_MYSQL_H
