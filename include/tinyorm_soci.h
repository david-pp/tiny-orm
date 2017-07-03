#ifndef TINYWORLD_TINYORM_SOCI_H
#define TINYWORLD_TINYORM_SOCI_H

#include <vector>
#include <set>
#include <unordered_set>
#include <functional>
#include "tinyorm.h"
#include "tinydb.h"

class TinySociORM {
public:
    typedef SOCIPool      PoolType;
    typedef soci::session ConnectionType;

    //
    // 构造: 支持两种方式:
    //   1. 连接池
    //   2. 指定连接
    //
    TinySociORM(SOCIPool *pool = &SOCIPool::instance())
            : pool_(pool) {
        if (pool_ && pool_->pool()) {
            session_ = new soci::session(*pool->pool());
        }
    }

    TinySociORM(soci::session *session)
            : session_(session) {
        pool_ = nullptr;
    }

    //
    // 析构: 如果是用连接池初始化的则把连接放回
    //
    ~TinySociORM() {
        if (pool_ && session_)
            delete session_;
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

protected:
    bool updateExistTable(TableDescriptorBase* td);

    bool createTable(TableDescriptorBase* td);

    bool dropTable(TableDescriptorBase* td);

    bool updateTable(TableDescriptorBase* td);

    struct UseResultBase {};

    template <typename UseT>
    struct UseResult : public UseResultBase {
        UseResult() {}
        UseResult(const UseT& v) : value(v) {}
        virtual ~UseResult() {}
        UseT value;
    };

    template<typename T>
    UseResultBase* fieldToStatement(soci::statement &st, T &obj, TableDescriptor<T> *td, FieldDescriptor::Ptr fd);

    template<typename T>
    bool recordToObject(soci::row &record, T &obj, TableDescriptor<T> *td);

private:
    soci::session &session() { return *session_; }

    SOCIPool *pool_ = nullptr;
    soci::session *session_ = nullptr;
};


#include "tinyorm_soci.in.h"

using TinyORM = TinySociORM;

template <typename T>
using Object2DB = Object2DB_T<T, TinySociORM>;

#endif //TINYWORLD_TINYORM_SOCI_H
