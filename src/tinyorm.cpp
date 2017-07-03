#include "tinyorm.h"

std::string FieldDescriptor::sql_type() {
    switch (type) {
        case FieldType::INT8   :
            return "TINYINT(3)";
        case FieldType::INT16  :
            return "SMALLINT(5)";
        case FieldType::INT32  :
            return "INT(10)";
        case FieldType::INT64  :
            return "BIGINT(20)";

        case FieldType::UINT8  :
            return "TINYINT(3) UNSIGNED";
        case FieldType::UINT16 :
            return "SMALLINT(5) UNSIGNED";
        case FieldType::UINT32 :
            return "INT(10) UNSIGNED";
        case FieldType::UINT64 :
            return "BIGINT(20) UNSIGNED";

        case FieldType::BOOL   :
            return "BOOL";

        case FieldType::FLOAT  :
            return "FLOAT";
        case FieldType::DOUBLE :
            return "DOUBLE";

        case FieldType::STRING :
            return "TEXT";
        case FieldType::VCHAR  : {
            char vchar[32] = "";
            snprintf(vchar, sizeof(vchar), "VARCHAR(%u)", size);
            return vchar;
        }

        case FieldType::BYTES:
            return "BLOB";
        case FieldType::BYTES_TINY:
            return "TINYBLOB";
        case FieldType::BYTES_MEDIUM:
            return "MEDIUMBLOB";
        case FieldType::BYTES_LONG:
            return "LONGBLOB";
        case FieldType::OBJECT:
            return "BLOB";
    }
    return "";
}

std::string FieldDescriptor::sql_default() {
    if (deflt.empty()) {
        if (FieldType::INT8 == type ||
            FieldType::INT16 == type ||
            FieldType::INT32 == type ||
            FieldType::INT64 == type ||
            FieldType::UINT8 == type ||
            FieldType::UINT16 == type ||
            FieldType::UINT32 == type ||
            FieldType::UINT64 == type ||
            FieldType::BOOL == type ||
            FieldType::FLOAT == type ||
            FieldType::DOUBLE == type)
            return "0";
    }
    return deflt;
}

std::string FieldDescriptor::sql_ddl() {
    std::ostringstream os;
    os << "`" << name << "`"
       << " " << sql_type();

    if (FieldType::STRING == type ||
            FieldType::BYTES == type ||
            FieldType::BYTES_TINY == type ||
            FieldType::BYTES_MEDIUM == type ||
            FieldType::BYTES_LONG == type ||
            FieldType::OBJECT == type )
        return os.str();

    os << " NOT NULL DEFAULT '" << sql_default() << "'";
    return os.str();
}

std::string TableDescriptorBase::sql_create() {
    std::ostringstream os;
    os << "CREATE TABLE `" << table << "` (";

    if (keys_.size()) {
        for (size_t i = 0; i < fields_ordered_.size(); ++i) {
            os << fields_ordered_[i]->sql_ddl() << ", ";
        }
        os << "PRIMARY KEY(";
        for (size_t i = 0; i < keys_.size(); ++i) {
            os << "`" << keys_[i]->name << "`";
            if (i != (keys_.size() - 1))
                os << ",";
        }
        os << ")";

    } else {
        for (size_t i = 0; i < fields_ordered_.size(); ++i) {
            os << fields_ordered_[i]->sql_ddl();
            if (i != (fields_ordered_.size() - 1))
                os << ", ";
        }
    }

    os << ")";
    return os.str();
}

std::string TableDescriptorBase::sql_drop() {
    return std::string("DROP TABLE `") + table + "`";
}

std::string TableDescriptorBase::sql_addfield(const std::string &name) {
    std::ostringstream os;
    auto fd = getFieldDescriptor(name);
    if (fd) {
        os << "ALTER TABLE `" << table << "` ADD "
           << fd->sql_ddl();
    }
    return os.str();
}

std::string TableDescriptorBase::sql_fieldlist() {
    std::ostringstream os;
    for (size_t i = 0; i < fields_ordered_.size(); ++i)
    {
        os << "`" << fields_ordered_[i]->name << "`";
        if (i != (fields_ordered_.size() - 1))
            os << ",";
    }
    return os.str();
}

std::string TableDescriptorBase::sql_fieldlist2(){
    std::ostringstream os;
    for (size_t i = 0; i < fields_ordered_.size(); ++i)
    {
        os << ":" << fields_ordered_[i]->name << "";
        if (i != (fields_ordered_.size() - 1))
            os << ",";
    }
    return os.str();
}


TableDescriptorBase &TableDescriptorBase::field(const std::string &name, FieldType type,
                                        const std::string &deflt, size_t size) {

    FieldDescriptor::Ptr fd(new FieldDescriptor(name, type, deflt, size));
    fields_[name] = fd;
    fields_ordered_.push_back(fd);
    return *this;
}

TableDescriptorBase &TableDescriptorBase::key(const std::string &name) {
    auto fd = getFieldDescriptor(name);
    if (fd) {
        keys_.push_back(fd);
    }
    return *this;
}

TableDescriptorBase &TableDescriptorBase::keys(const std::initializer_list<std::string> &names) {
    for (auto name : names)
        key(name);
    return *this;
}

TableDescriptorBase &TableDescriptorBase::index(const std::string &name) {
    auto fd = getFieldDescriptor(name);
    if (fd) {
        indexs_.push_back(fd);
    }
    return *this;
}

TableDescriptorBase &TableDescriptorBase::indexs(const std::initializer_list<std::string> &names) {
    for (auto name : names)
        index(name);
    return *this;
}

FieldDescriptor::Ptr TableDescriptorBase::getFieldDescriptor(const std::string &name) {
    auto it = fields_.find(name);
    if (it != fields_.end())
        return it->second;
    return nullptr;
}