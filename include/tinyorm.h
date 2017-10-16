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

#ifndef TINYWORLD_TINYORM_H
#define TINYWORLD_TINYORM_H

#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>
#include <memory>
#include "tinyreflection.h"
#include "tinyserializer.h"
#include "tinyserializer_proto.h"

enum class FieldType : uint8_t {
    INT8,
    INT16,
    INT32,
    INT64,

    UINT8,
    UINT16,
    UINT32,
    UINT64,

    BOOL,

    FLOAT,
    DOUBLE,

    STRING,
    VCHAR,

    BYTES,
    BYTES_TINY,
    BYTES_MEDIUM,
    BYTES_LONG,

    OBJECT,
};

class FieldDescriptor {
public:
    using Ptr = std::shared_ptr<FieldDescriptor>;

    FieldDescriptor(const std::string &_name,
                    FieldType _type,
                    const std::string &_deflt,
                    size_t _size)
            : name(_name), type(_type), deflt(_deflt), size(_size) {}

    std::string sql_ddl();

    std::string sql_type();

    std::string sql_default();

    // Field name
    std::string name;
    // Field type
    FieldType type;
    // Default value
    std::string deflt;
    // Field size (some type is valid)
    uint32_t size;
};

using FieldDescriptorList = std::vector<FieldDescriptor::Ptr>;

class TableDescriptorBase {
public:
    using Ptr = std::shared_ptr<TableDescriptorBase>;

    std::string sql_create();

    std::string sql_drop();

    std::string sql_addfield(const std::string &field);

    std::string sql_fieldlist();

    std::string sql_fieldlist2();

public:
    TableDescriptorBase &field(const std::string &name,
                               FieldType type,
                               const std::string &deflt = "",
                               size_t size = 0);

    TableDescriptorBase &key(const std::string &name);

    TableDescriptorBase &keys(const std::initializer_list<std::string> &names);

    TableDescriptorBase &index(const std::string &name);

    TableDescriptorBase &indexs(const std::initializer_list<std::string> &names);

    FieldDescriptor::Ptr getFieldDescriptor(const std::string &name);

    const FieldDescriptorList &fields() { return fields_ordered_; }

    const FieldDescriptorList &keys() { return keys_; }

public:
    TableDescriptorBase(const std::string name)
            : table(name) {}

    // Talbe name
    std::string table;
    // Primary keys
    FieldDescriptorList keys_;
    // Indexes
    FieldDescriptorList indexs_;
    // Field Descriptors
    FieldDescriptorList fields_ordered_;
    // Field Descriptors by name
    std::unordered_map<std::string, FieldDescriptor::Ptr> fields_;
};


template<typename T>
class TableDescriptor : public TableDescriptorBase {
public:
    TableDescriptor(const std::string &name)
            : TableDescriptorBase(name), reflection(name) {}

    template<template<typename> class SerializerT = ProtoSerializer, typename PropType>
    TableDescriptor<T> &field(PropType T::* prop,
                              const std::string &name,
                              FieldType type,
                              const std::string &deflt = "",
                              size_t size = 0) {
        reflection.template property<SerializerT>(name, prop);
        TableDescriptorBase::field(name, type, deflt, size);
        return *this;
    }

    Struct<T> reflection;
};

class TableFactory {
public:
    typedef std::unordered_map<std::string, TableDescriptorBase::Ptr> Tables;

    static TableFactory &instance() {
        static TableFactory factory_;
        return factory_;
    }

    template<typename T>
    TableDescriptor<T> &table(const std::string &name) {
        TableDescriptor<T> *td = new TableDescriptor<T>(name);
        TableDescriptorBase::Ptr ptr(td);
        tables_byname_[name] = ptr;
        tables_bytype_[typeid(T).name()] = ptr;
        return *td;
    }

    TableDescriptorBase::Ptr tableByName(const std::string &name) {
        auto it = tables_byname_.find(name);
        if (it != tables_byname_.end())
            return it->second;
        return nullptr;
    }

    template<typename T>
    TableDescriptor<T> *tableByType() {
        auto it = tables_bytype_.find(typeid(T).name());
        if (it != tables_bytype_.end())
            return static_cast<TableDescriptor<T> *>(it->second.get());
        return nullptr;
    }

    Tables &tables() { return tables_byname_; }

private:
    Tables tables_byname_;
    Tables tables_bytype_;
};


//
// Helper Class: Add datatabase operations to the object.
// eg.
//   Object2DB_T<Object, ORM> object;
//   object.selectDB();
//   object.updateDB();
//
// ORM :
//   - TinyMySqlORM
//   - TinySociORM
//
template<typename T, typename ORM>
struct Object2DB_T : public T {
public:
    using Records = std::vector<std::shared_ptr<T>>;
    using ORMPoolType = typename ORM::PoolType;
    using ORMConnectionType = typename ORM::ConnectionType;

    Object2DB_T() {}

    Object2DB_T(const T &object) : T(object) {}

    //
    // Create, Drop and Update Table's Structure
    //
    static bool createTable(ORMPoolType *pool = &ORMPoolType::instance()) {
        ORM orm(pool);
        return orm.template createTableByType<T>();
    }

    static bool createTable(ORMConnectionType *connect) {
        ORM orm(connect);
        return orm.template createTableByType<T>();
    }

    static bool dropTable(ORMPoolType *pool = &ORMPoolType::instance()) {
        ORM orm(pool);
        return orm.template dropTableByType<T>();
    }

    static bool dropTable(ORMConnectionType *connect) {
        ORM orm(connect);
        return orm.template dropTableByType<T>();
    }


    static bool updateTable(ORMPoolType *pool = &ORMPoolType::instance()) {
        ORM orm(pool);
        return orm.template updateTableByType<T>();
    }

    static bool updateTable(ORMConnectionType *connect) {
        ORM orm(connect);
        return orm.template updateTableByType<T>();
    }

public:
    //
    // Database Operations
    //
    bool selectDB(ORMPoolType *pool = &ORMPoolType::instance()) {
        ORM orm(pool);
        return orm.template select<T>(*this);
    }

    bool selectDB(ORMConnectionType *connect) {
        ORM orm(connect);
        return orm.template select<T>(*this);
    }


    bool insertDB(ORMPoolType *pool = &ORMPoolType::instance()) {
        ORM orm(pool);
        return orm.template insert<T>(*this);
    }

    bool insertDB(ORMConnectionType *connect) {
        ORM orm(connect);
        return orm.template insert<T>(*this);
    }

    bool replaceDB(ORMPoolType *pool = &ORMPoolType::instance()) {
        ORM orm(pool);
        return orm.template replace<T>(*this);
    }

    bool replaceDB(ORMConnectionType *connect) {
        ORM orm(connect);
        return orm.template replace<T>(*this);
    }

    bool updateDB(ORMPoolType *pool = &ORMPoolType::instance()) {
        ORM orm(pool);
        return orm.template update<T>(*this);
    }

    bool updateDB(ORMConnectionType *connect) {
        ORM orm(connect);
        return orm.template update<T>(*this);
    }

    bool deleteDB(ORMPoolType *pool = &ORMPoolType::instance()) {
        ORM orm(pool);
        return orm.template del<T>(*this);
    }

    bool deleteDB(ORMConnectionType *connect) {
        ORM orm(connect);
        return orm.template del<T>(*this);
    }
};


#endif //TINYWORLD_TINYORM_H
