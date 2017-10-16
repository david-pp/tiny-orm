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

#ifndef TINYWORLD_TINYORM_SOCI_IN_H
#define TINYWORLD_TINYORM_SOCI_IN_H

bool TinySociORM::showTables(std::unordered_set<std::string> &tables) {

    try {
        soci::rowset<soci::row> show = (session_->prepare << "SHOW TABLES");
        for (auto it = show.begin(); it != show.end(); ++it) {
            std::string name = it->get<std::string>(0);
            std::for_each(name.begin(), name.end(), [](char &c) {
                c = std::toupper(c);
            });
            tables.insert(name);
        }
    }
    catch (std::exception &e) {
        LOG_ERROR("TinySociORM", "showTables, error:%s", e.what());
        return false;
    }

    return true;
}

bool TinySociORM::updateTables() {

    std::unordered_set<std::string> tables;
    if (!showTables(tables))
        return false;

    for (auto it = TableFactory::instance().tables().begin();
         it != TableFactory::instance().tables().end(); ++it) {

        if (tables.find(it->second->table) == tables.end()) { // not exist
            createTableByName(it->second->table);
        } else {
            updateExistTable(it->second.get());
        }
    }

    return true;
}

template<typename T>
bool TinySociORM::createTableByType() {
    auto td = TableFactory::instance().tableByType<T>();
    if (!td) {
        LOG_ERROR("TinySociORM", "%s: Table descriptor is not exist", __PRETTY_FUNCTION__);
        return false;
    }

    return createTable(td);
}

bool TinySociORM::createTableByName(const std::string &name) {
    auto td = TableFactory::instance().tableByName(name);
    if (!td) {
        LOG_ERROR("TinySociORM", "%s: Table descriptor is not exist", __PRETTY_FUNCTION__);
        return false;
    }

    return createTable(td.get());
}


template<typename T>
bool TinySociORM::dropTableByType() {
    auto td = TableFactory::instance().tableByType<T>();
    if (!td) {
        LOG_ERROR("TinySociORM", "%s: Table descriptor is not exist", __PRETTY_FUNCTION__);
        return false;
    }

    return dropTable(td);
}

bool TinySociORM::dropTableByName(const std::string &name) {
    auto td = TableFactory::instance().tableByName(name);
    if (!td) {
        LOG_ERROR("TinySociORM", "%s: Table descriptor is not exist", __PRETTY_FUNCTION__);
        return false;
    }

    return dropTable(td.get());
}


template<typename T>
bool TinySociORM::updateTableByType() {
    auto td = TableFactory::instance().tableByType<T>();
    if (!td) {
        LOG_ERROR("TinySociORM", "%s: Table descriptor is not exist", __PRETTY_FUNCTION__);
        return false;
    }

    return updateTable(td);
}

bool TinySociORM::updateTableByName(const std::string &name) {
    auto td = TableFactory::instance().tableByName(name);
    if (!td) {
        LOG_ERROR("TinySociORM", "%s: Table descriptor is not exist", __PRETTY_FUNCTION__);
        return false;
    }

    return updateTable(td.get());
}

bool TinySociORM::updateExistTable(TableDescriptorBase *td) {

    try {
        std::set<std::string> fields_db;
        soci::rowset<soci::row> desc = session_->prepare << "DESC `" << td->table << "`";
        for (auto it = desc.begin(); it != desc.end(); ++it) {
            fields_db.insert(it->get<std::string>(0));
        }

        for (auto fd : td->fields()) {
            // need update
            if (fields_db.find(fd->name) == fields_db.end()) {
                (*session_) << td->sql_addfield(fd->name);
            }
        }
        return true;
    }
    catch (std::exception &e) {
        LOG_ERROR("TinyMySqlORM", "updateExistTable:%s, error:%s", td->table.c_str(), e.what());
        return false;
    }
}

bool TinySociORM::updateTable(TableDescriptorBase *td) {
    if (!td) return false;

    try {
        soci::rowset<soci::row> desc = session_->prepare << "SHOW TABLES LIKE \"" << td->table << "\"";
        if (desc.begin() != desc.end())  // table exist
            return updateExistTable(td);
        else  // table not exist then create
            return createTable(td);
    }
    catch (std::exception &e) {
        LOG_ERROR("TinySociORM", "updateTable:%s, error:%s", td->table.c_str(), e.what());
        return false;
    }

    return false;
}

bool TinySociORM::createTable(TableDescriptorBase* td) {
    if (!td) return false;

    try {
        session() << td->sql_create();
    }
    catch (std::exception &e) {
        LOG_ERROR("TinySociORM", "createTable:%s, error:%s", td->table.c_str(), e.what());
        return false;
    }

    return true;
}

bool TinySociORM::dropTable(TableDescriptorBase* td) {
    if (!td) return false;

    try {
        session() << td->sql_drop();
    }
    catch (std::exception &e) {
        LOG_ERROR("TinySociORM", "dropTable:%s, error:%s", td->table.c_str(), e.what());
        return false;
    }

    return true;
}


template<typename T>
bool TinySociORM::insert(T &obj) {

    auto td = TableFactory::instance().tableByType<T>();
    if (!td) {
        LOG_ERROR("TinySociORM", "%s: Table descriptor is not exist", __PRETTY_FUNCTION__);
        return false;
    }

    try {
        std::ostringstream os;
        os << "INSERT INTO `" << td->table
           << "`(" << td->sql_fieldlist() << ")"
           << " VALUES (" << td->sql_fieldlist2() << ")";

        soci::statement st(*session_);

        // 由于soci::use传引用, 所以必须临时存一下成员变量为C++对象时序列化的结果
        std::vector<std::shared_ptr<UseResultBase>> values;

        for (auto fd : td->fields()) {
            std::shared_ptr<UseResultBase> result(fieldToStatement<T>(st, obj, td, fd));
            if (result) {
                values.push_back(result);
            }
        }

        st.alloc();
        st.prepare(os.str());
        st.define_and_bind();
        st.execute(true);

        return true;
    }
    catch (std::exception &err) {
        LOG_ERROR("TinySociORM", "%s: %s", __PRETTY_FUNCTION__, err.what());
        return false;
    }
}

template<typename T>
bool TinySociORM::select(T &obj) {

    auto td = TableFactory::instance().tableByType<T>();
    if (!td) {
        LOG_ERROR("TinySociORM", "%s: Table descriptor is not exist", __PRETTY_FUNCTION__);
        return false;
    }

    try {
        std::ostringstream os;
        os << "select " << td->sql_fieldlist() << " from `" << td->table << "` WHERE";

        size_t i = 0;
        for (auto fd : td->keys()) {
            os << "`" << fd->name << "`=:" << fd->name;
            i++;

            if (i != td->keys().size())
                os << " AND ";
        }


        soci::statement st(*session_);

        // 由于soci::use传引用, 所以必须临时存一下成员变量为C++对象时序列化的结果
        std::vector<std::shared_ptr<UseResultBase>> values;

        for (auto fd : td->keys()) {
            std::shared_ptr<UseResultBase> result(fieldToStatement<T>(st, obj, td, fd));
            if (result) {
                values.push_back(result);
            }
        }

        soci::row row;

        st.exchange(soci::into(row));

        st.alloc();
        st.prepare(os.str());
        st.define_and_bind();
        st.execute(false);

        while (st.fetch()) {
            return recordToObject(row, obj, td);
        }
        return false;
    }
    catch (std::exception &err) {
        LOG_ERROR("TinySociORM", "%s: %s", __PRETTY_FUNCTION__, err.what());
        return false;
    }

    return false;
}

template<typename T>
bool TinySociORM::replace(T &obj) {

    auto td = TableFactory::instance().tableByType<T>();
    if (!td) {
        LOG_ERROR("TinySociORM", "%s: Table descriptor is not exist", __PRETTY_FUNCTION__);
        return false;
    }

    try {
        std::ostringstream os;
        os << "REPLACE INTO `" << td->table
           << "`(" << td->sql_fieldlist() << ")"
           << " VALUES (" << td->sql_fieldlist2() << ")";

        soci::statement st(*session_);

        // 由于soci::use传引用, 所以必须临时存一下成员变量为C++对象时序列化的结果
        std::vector<std::shared_ptr<UseResultBase>> values;

        for (auto fd : td->fields()) {
            std::shared_ptr<UseResultBase> result(fieldToStatement<T>(st, obj, td, fd));
            if (result) {
                values.push_back(result);
            }
        }

        st.alloc();
        st.prepare(os.str());
        st.define_and_bind();
        st.execute(true);

        return true;
    }
    catch (std::exception &err) {
        LOG_ERROR("TinySociORM", "%s: %s", __PRETTY_FUNCTION__, err.what());
        return false;
    }
}

template<typename T>
bool TinySociORM::update(T &obj) {

    auto td = TableFactory::instance().tableByType<T>();
    if (!td) {
        LOG_ERROR("TinySociORM", "%s: Table descriptor is not exist", __PRETTY_FUNCTION__);
        return false;
    }

    try {
        std::ostringstream os;
        os << "UPDATE `" << td->table << "` SET ";

        size_t i = 0;
        for (auto fd : td->fields()) {
            os << "`" << fd->name << "`=:" << fd->name;
            i++;

            if (i != td->fields().size())
                os << ",";
        }

        os << " WHERE ";

        i = 0;
        for (auto fd : td->keys()) {
            os << "`" << fd->name << "`=:" << fd->name;
            i++;

            if (i != td->keys().size())
                os << " AND ";
        }


        soci::statement st(*session_);

        // 由于soci::use传引用, 所以必须临时存一下成员变量为C++对象时序列化的结果
        std::vector<std::shared_ptr<UseResultBase>> values;

        for (auto fd : td->fields()) {
            std::shared_ptr<UseResultBase> result(fieldToStatement<T>(st, obj, td, fd));
            if (result) {
                values.push_back(result);
            }
        }

        for (auto fd : td->keys()) {
            std::shared_ptr<UseResultBase> result(fieldToStatement<T>(st, obj, td, fd));
            if (result) {
                values.push_back(result);
            }
        }

        st.alloc();
        st.prepare(os.str());
        st.define_and_bind();
        st.execute(true);

        return true;
    }
    catch (std::exception &err) {
        LOG_ERROR("TinySociORM", "%s: %s", __PRETTY_FUNCTION__, err.what());
        return false;
    }
}

template<typename T>
bool TinySociORM::del(T &obj) {

    auto td = TableFactory::instance().tableByType<T>();
    if (!td) {
        LOG_ERROR("TinySociORM", "%s: Table descriptor is not exist", __PRETTY_FUNCTION__);
        return false;
    }

    try {
        std::ostringstream os;
        os << "DELETE FROM `" << td->table << "` WHERE ";

        size_t i = 0;
        for (auto fd : td->keys()) {
            os << "`" << fd->name << "`=:" << fd->name;
            i++;

            if (i != td->keys().size())
                os << " AND ";
        }

        soci::statement st(*session_);

        // 由于soci::use传引用, 所以必须临时存一下成员变量为C++对象时序列化的结果
        std::vector<std::shared_ptr<UseResultBase>> values;

        for (auto fd : td->keys()) {
            std::shared_ptr<UseResultBase> result(fieldToStatement<T>(st, obj, td, fd));
            if (result) {
                values.push_back(result);
            }
        }

        st.alloc();
        st.prepare(os.str());
        st.define_and_bind();
        st.execute(true);

        return true;
    }
    catch (std::exception &err) {
        LOG_ERROR("TinySociORM", "%s: %s", __PRETTY_FUNCTION__, err.what());
        return false;
    }
    return false;
}


template<typename T>
bool TinySociORM::vloadFromDB(const std::function<void(std::shared_ptr<T>)> &callback, const char *clause, va_list ap) {
    auto td = TableFactory::instance().tableByType<T>();
    if (!td) {
        LOG_ERROR("TinySociORM", "%s: Table descriptor is not exist", __PRETTY_FUNCTION__);
        return false;
    }

    char statement[1024] = "";
    if (clause) {
        vsnprintf(statement, sizeof(statement), clause, ap);
    }

    try {
        soci::rowset<soci::row> records = (session_->prepare << "SELECT " << td->sql_fieldlist() << " FROM `"
                                                             << td->table << "` " << statement);

        for (auto it = records.begin(); it != records.end(); ++it) {
            std::shared_ptr<T> obj = std::make_shared<T>();
            if (recordToObject(*it, *obj.get(), td)) {
                callback(obj);
            } else {
                LOG_ERROR("TinySociORM", "%s: recordToObject FAILED", __PRETTY_FUNCTION__);
            }
        }
    }
    catch (std::exception &err) {
        LOG_ERROR("TinySociORM", "%s: %s", __PRETTY_FUNCTION__, err.what());
        return false;
    }

    return false;
}

template<typename T>
bool TinySociORM::loadFromDB(const std::function<void(std::shared_ptr<T>)> &callback, const char *clause, ...) {
    va_list ap;
    va_start(ap, clause);
    bool ret = vloadFromDB<T>(callback, clause, ap);
    va_end(ap);

    return ret;
}

template<typename T>
bool TinySociORM::loadFromDB(Records <T> &records, const char *clause, ...) {

    va_list ap;
    va_start(ap, clause);

    bool ret = vloadFromDB<T>([&records](std::shared_ptr<T> record) {
        records.push_back(record);
    }, clause, ap);

    va_end(ap);
    return ret;
}

template<typename T, typename TSet>
bool TinySociORM::loadFromDB(TSet &records, const char *clause, ...) {
    va_list ap;
    va_start(ap, clause);

    bool ret = vloadFromDB<T>([&records](std::shared_ptr<T> record) {
        records.insert(record);
    }, clause, ap);

    va_end(ap);
    return ret;
};

template<typename T, typename TMultiIndexSet>
bool TinySociORM::loadFromDB2MultiIndexSet(TMultiIndexSet &records, const char *clause, ...) {
    va_list ap;
    va_start(ap, clause);

    bool ret = vloadFromDB<T>([&records](std::shared_ptr<T> record) {
        records.insert(*record);
    }, clause, ap);

    va_end(ap);
    return ret;
};

template<typename T>
bool TinySociORM::deleteFromDB(const char *where, ...) {
    auto td = TableFactory::instance().tableByType<T>();
    if (!td) {
        LOG_ERROR("TinySociORM", "%s: Table descriptor is not exist", __PRETTY_FUNCTION__);
        return false;
    }

    char statement[1024] = "";
    if (where) {
        va_list ap;
        va_start(ap, where);
        vsnprintf(statement, sizeof(statement), where, ap);
        va_end(ap);
    }

    try {
        session() << "DELETE FROM `" << td->table << "` " << statement;
        return true;
    }
    catch (std::exception &err) {
        LOG_ERROR("TinyMySqlORM", "%s: %s", __PRETTY_FUNCTION__, err.what());
        return false;
    }

    return false;
}


template<typename T>
TinySociORM::UseResultBase *
TinySociORM::fieldToStatement(soci::statement &st, T &obj, TableDescriptor<T> *td, FieldDescriptor::Ptr fd) {
    if (!td || !fd) return nullptr;

    // http://soci.sourceforge.net/doc/3.2/backends/mysql.html#dynamic
    switch (fd->type) {

        case FieldType::INT8   : {
            auto result = new UseResult<int>(td->reflection.template get<int8_t>(obj, fd->name));
            st.exchange(soci::use(result->value));
            return result;
        }
        case FieldType::INT16  : {
            auto result = new UseResult<int>(td->reflection.template get<int16_t>(obj, fd->name));
            st.exchange(soci::use(result->value));
            return result;
        }
        case FieldType::INT32  : {
            auto result = new UseResult<int>(td->reflection.template get<int32_t>(obj, fd->name));
            st.exchange(soci::use(result->value));
            return result;
        }
        case FieldType::INT64  : {
            auto result = new UseResult<long long>(td->reflection.template get<int64_t>(obj, fd->name));
            st.exchange(soci::use(result->value));
            return result;
        }
        case FieldType::UINT8   : {
            auto result = new UseResult<int>(td->reflection.template get<uint8_t>(obj, fd->name));
            st.exchange(soci::use(result->value));
            return result;
        }
        case FieldType::UINT16  : {
            auto result = new UseResult<int>(td->reflection.template get<uint16_t>(obj, fd->name));
            st.exchange(soci::use(result->value));
            return result;
        }
        case FieldType::UINT32  : {
            auto result = new UseResult<long long>(td->reflection.template get<uint32_t>(obj, fd->name));
            st.exchange(soci::use(result->value));
            return result;
        }
        case FieldType::UINT64  : {
            auto result = new UseResult<unsigned long long>(td->reflection.template get<uint64_t>(obj, fd->name));
            st.exchange(soci::use(result->value));
            return result;
        }

        case FieldType::BOOL   : {
            auto result = new UseResult<int>(td->reflection.template get<bool>(obj, fd->name));
            st.exchange(soci::use(result->value));
            return result;
        }

        case FieldType::FLOAT  : {
            auto result = new UseResult<double>(td->reflection.template get<float>(obj, fd->name));
            st.exchange(soci::use(result->value));
            return result;
        }
        case FieldType::DOUBLE : {
            auto result = new UseResult<double>(td->reflection.template get<double>(obj, fd->name));
            st.exchange(soci::use(result->value));
            return result;
        }

        case FieldType::STRING : {
            auto result = new UseResult<std::string>(td->reflection.template get<std::string>(obj, fd->name));
            st.exchange(soci::use(result->value));
            return result;
        }
        case FieldType::VCHAR  : {
            auto result = new UseResult<std::string>(td->reflection.template get<std::string>(obj, fd->name));
            st.exchange(soci::use(result->value));
            return result;
        }

        case FieldType::BYTES: {
            auto result = new UseResult<std::string>(td->reflection.template get<std::string>(obj, fd->name));
            st.exchange(soci::use(result->value));
            return result;
        }
        case FieldType::BYTES_TINY: {
            auto result = new UseResult<std::string>(td->reflection.template get<std::string>(obj, fd->name));
            st.exchange(soci::use(result->value));
            return result;
        }
        case FieldType::BYTES_MEDIUM: {
            auto result = new UseResult<std::string>(td->reflection.template get<std::string>(obj, fd->name));
            st.exchange(soci::use(result->value));
            return result;
        }
        case FieldType::BYTES_LONG: {
            auto result = new UseResult<std::string>(td->reflection.template get<std::string>(obj, fd->name));
            st.exchange(soci::use(result->value));
            return result;
        }
        case FieldType::OBJECT: {
            auto pd = td->reflection.propertyByName(fd->name);
            if (pd) {
                auto result = new UseResult<std::string>();
                result->value = pd->serialize(obj);
                st.exchange(soci::use(result->value));
                return result;

            } else {
                return nullptr;
            }
        }
    }

    return nullptr;
}

template<typename T>
bool TinySociORM::recordToObject(soci::row &record, T &obj, TableDescriptor<T> *td) {
    if (!td || record.size() < td->fields().size())
        return false;

    bool ret = true;

    // http://soci.sourceforge.net/doc/3.2/backends/mysql.html#dynamic
    for (size_t i = 0; i < td->fields().size(); ++i) {
        auto fd = td->fields().at(i);
//        LOG_TRACE("TinySociORM", "field:%s", fd->name.c_str());
        switch (fd->type) {

            case FieldType::INT8   : {
                td->reflection.template set<int8_t>(obj, fd->name, record.get<int>(i));
                break;
            }
            case FieldType::INT16  : {
                td->reflection.template set<int16_t>(obj, fd->name, record.get<int>(i));
                break;
            }
            case FieldType::INT32  : {
                td->reflection.template set<int32_t>(obj, fd->name, record.get<int>(i));
                break;
            }
            case FieldType::INT64  : {
                td->reflection.template set<int64_t>(obj, fd->name, record.get < long
                long > (i));
                break;
            }
            case FieldType::UINT8   : {
                td->reflection.template set<uint8_t>(obj, fd->name, record.get<int>(i));
                break;
            }
            case FieldType::UINT16  : {
                td->reflection.template set<uint16_t>(obj, fd->name, record.get<int>(i));
                break;
            }
            case FieldType::UINT32  : {
                td->reflection.template set<uint32_t>(obj, fd->name, record.get < long
                long > (i));
                break;
            }
            case FieldType::UINT64  : {
                td->reflection.template set<uint64_t>(obj, fd->name, record.get < unsigned
                long
                long > (i));
                break;
            }

            case FieldType::BOOL   : {
                td->reflection.template set<bool>(obj, fd->name, record.get<int>(i));
                break;
            }

            case FieldType::FLOAT  : {
                td->reflection.template set<float>(obj, fd->name, record.get<double>(i));
                break;
            }
            case FieldType::DOUBLE : {
                td->reflection.template set<double>(obj, fd->name, record.get<double>(i));
                break;
            }

            case FieldType::STRING : {
                td->reflection.template set<std::string>(obj, fd->name, record.get<std::string>(i));
                break;
            }
            case FieldType::VCHAR  : {
                td->reflection.template set<std::string>(obj, fd->name, record.get<std::string>(i));
                break;
            }

            case FieldType::BYTES: {
                td->reflection.template set<std::string>(obj, fd->name, record.get<std::string>(i));
                break;
            }
            case FieldType::BYTES_TINY: {
                td->reflection.template set<std::string>(obj, fd->name, record.get<std::string>(i));
                break;
            }
            case FieldType::BYTES_MEDIUM: {
                td->reflection.template set<std::string>(obj, fd->name, record.get<std::string>(i));
                break;
            }
            case FieldType::BYTES_LONG: {
                td->reflection.template set<std::string>(obj, fd->name, record.get<std::string>(i));
                break;
            }
            case FieldType::OBJECT: {
                auto prop = td->reflection.propertyByName(fd->name);
                if (!prop) {
                    ret = false;
                    LOG_ERROR("TinySociORM", "%s.%s Property Reflection is not exist", td->table.c_str(),
                              fd->name.c_str());
                    break;
                }
                if (!prop->deserialize(obj, record.get<std::string>(i))) {
                    ret = false;
                    LOG_ERROR("TinySociORM", "%s.%s Property deserialize failed", td->table.c_str(), fd->name.c_str());
                    break;
                }
            }
        }
    }

    return ret;
}


#endif //TINYWORLD_TINYORM_SOCI_IN_H
