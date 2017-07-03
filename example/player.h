#ifndef TINYWORLD_PLAYER_H
#define TINYWORLD_PLAYER_H

#include "tinyworld.h"
#include "tinyorm.h"
#include "player.pb.h"

struct Weapon {
    uint32_t type = 0;
    std::string name = "";

    std::string serialize() const {
        WeaponProto proto;
        proto.set_type(type);
        proto.set_name(name);
        return proto.SerializeAsString();
    }

    bool deserialize(const std::string &data) {
        WeaponProto proto;
        if (proto.ParseFromString(data)) {
            type = proto.type();
            name = proto.name();
            return true;
        }
        return false;
    }
};

//
// Player -> PLAYER
//
struct Player {
    //
    // TableMeta约束
    //
    typedef uint32_t TableKey;

    static const char *tableName() { return "player"; }

    TableKey tableKey() const { return id; }
    void tableKey(const TableKey& key) { id = key; }

    //
    // Proto序列化约束
    //
    std::string serialize() const {
        PlayerProto4Table proto;
        proto.set_id(id);
        proto.set_name(name);
        proto.set_age(age);
        proto.set_weaon(::serialize(weapon));
        proto.set_weapns(::serialize(weapons));
        return proto.SerializeAsString();
    }

    bool deserialize(const std::string &data) {
        PlayerProto4Table proto;
        if (proto.ParseFromString(data)) {
            id = proto.id();
            name = proto.name();
            age = proto.age();
            ::deserialize(weapon, proto.weaon());
            ::deserialize(weapons, proto.weapns());

            return true;
        }
        return false;
    }

    // ...
    uint32_t id = 0;
    std::string name;
    uint8_t age = 0;

    // 整型
    int8_t m_int8 = 0;
    uint8_t m_uint8 = 0;
    int16_t m_int16 = 0;
    uint16_t m_uint16 = 0;
    int32_t m_int32 = 0;
    uint32_t m_uint32 = 0;
    int64_t m_int64 = 0;
    uint64_t m_uint64 = 0;

    // 浮点
    float m_float = 0.0;
    double m_double = 0.0;

    // 布尔
    bool m_bool = false;

    // 字符串
    std::string m_string;

    // 二进制
    std::string m_bytes;
    std::string m_bytes_tiny;
    std::string m_bytes_medium;
    std::string m_bytes_long;

    // 复合对象
    Weapon weapon;
    std::vector<Weapon> weapons;

    void init() {
        id = 1024;
        name = "david";
        age = 30;

        // 整型
        m_int8 = 1;
        m_uint8 = 2;
        m_int16 = 3;
        m_uint16 = 4;
        m_int32 = 5;
        m_uint32 = 6;
        m_int64 = 7;
        m_uint64 = 8;
        // 浮点
        m_float = 10.11;
        m_double = 20.22;
        m_bool = true;

        m_string = "string";


        m_bytes = "bytes";
        m_bytes_tiny = "tiny...bytes";
        m_bytes_medium = "medium...bytes";
        m_bytes_long = "long...bytes";
        weapon.type = 24;
        weapon.name = "sword";
        weapons = {weapon, weapon, weapon};
    }
};

// 在cpp中定义一次即可
RUN_ONCE(Player) {
    TableFactory::instance().table<Player>("PLAYER")
            .field(&Player::id, "ID", FieldType::UINT32)
            .field(&Player::name, "NAME", FieldType::VCHAR, "david", 32)
            .field(&Player::age, "AGE", FieldType::UINT8, "30")

            .field(&Player::m_int8, "M_INT8", FieldType::INT8)
            .field(&Player::m_int16, "M_INT16", FieldType::INT16)
            .field(&Player::m_int32, "M_INT32", FieldType::INT32)
            .field(&Player::m_int64, "M_INT64", FieldType::INT64)
            .field(&Player::m_uint8, "M_UINT8", FieldType::UINT8)
            .field(&Player::m_uint16, "M_UINT16", FieldType::UINT16)
            .field(&Player::m_uint32, "M_UINT32", FieldType::UINT32)
            .field(&Player::m_uint64, "M_UINT64", FieldType::UINT64)

            .field(&Player::m_bool, "M_BOOL", FieldType::BOOL)
            .field(&Player::m_float, "M_FLOAT", FieldType::FLOAT)
            .field(&Player::m_double, "M_DOUBLE", FieldType::DOUBLE)

            .field(&Player::m_string, "M_STRING", FieldType::STRING)

            .field(&Player::m_bytes, "M_BYTES", FieldType::BYTES)
            .field(&Player::m_bytes_tiny, "M_BYTES_S", FieldType::BYTES_TINY)
            .field(&Player::m_bytes_medium, "M_BYTES_M", FieldType::BYTES_MEDIUM)
            .field(&Player::m_bytes_long, "M_BYTES_L", FieldType::BYTES_LONG)

            .field(&Player::weapon, "OBJ1", FieldType::OBJECT)
            .field(&Player::weapons, "OBJ2", FieldType::OBJECT)
            .key("ID")
            .index("NAME");
}

std::ostream &operator<<(std::ostream &os, const Player &p) {
    os << "-------- Player:(" << p.id << "," << p.name << ")------" << std::endl;
    os << "- m_int8    = " << (int) p.m_int8 << std::endl;
    os << "- m_int16   = " << p.m_int16 << std::endl;
    os << "- m_int32   = " << p.m_int32 << std::endl;
    os << "- m_int64   = " << p.m_int64 << std::endl;
    os << "- m_uint8   = " << (int) p.m_uint8 << std::endl;
    os << "- m_uint16  = " << p.m_uint16 << std::endl;
    os << "- m_uint32  = " << p.m_uint32 << std::endl;
    os << "- m_uint64  = " << p.m_uint64 << std::endl;
    os << "- m_float   = " << p.m_float << std::endl;
    os << "- m_double  = " << p.m_double << std::endl;
    os << "- m_bool    = " << p.m_bool << std::endl;
    os << "- m_string  = " << p.m_string << std::endl;

    os << "- m_bytes        = " << p.m_bytes.size() << std::endl;
    os << "- m_bytes_tiny   = " << p.m_bytes_tiny.size() << std::endl;
    os << "- m_bytes_medium = " << p.m_bytes_medium.size() << std::endl;
    os << "- m_bytes_long   = " << p.m_bytes_long.size() << std::endl;

    os << "- weapon         = {" << p.weapon.type << "," << p.weapon.name << "}" << std::endl;
    os << "- weapons        = " << p.weapons.size() << std::endl;

    return os;
}

#endif //TINYWORLD_PLAYER_H