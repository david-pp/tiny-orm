#include "tinyplayer.h"

RUN_ONCE(PlayerORM) {
     using namespace tiny;
     TableFactory::instance().table<Player>("PLAYER")
          .field(&Player::id, "ID", FieldType::UINT32, "0")
          .field(&Player::name, "NAME", FieldType::STRING, "david")
          .field(&Player::country, "COUNTRY", FieldType::OBJECT)
          .field(&Player::friends, "FRIENDS", FieldType::OBJECT)
          .field(&Player::m_int8, "M_INT8", FieldType::INT8, "22")
          .field(&Player::m_int16, "M_INT16", FieldType::INT16, "0")
          .field(&Player::m_int32, "M_INT32", FieldType::INT32, "0")
          .field(&Player::m_int64, "M_INT64", FieldType::INT64, "0")
          .field(&Player::m_uint8, "M_UINT8", FieldType::UINT8, "0")
          .field(&Player::m_uint16, "M_UINT16", FieldType::UINT16, "0")
          .field(&Player::m_uint32, "M_UINT32", FieldType::UINT32, "0")
          .field(&Player::m_uint64, "M_UINT64", FieldType::UINT64, "0")
          .field(&Player::m_float, "M_FLOAT", FieldType::FLOAT, "0")
          .field(&Player::m_double, "M_DOUBLE", FieldType::DOUBLE, "0")
          .field(&Player::m_bool, "M_BOOL", FieldType::BOOL, "0")
          .field(&Player::m_string, "M_STRING", FieldType::STRING)
          .field(&Player::m_bytes, "M_BYTES", FieldType::BYTES)
          .field(&Player::m_bytes_tiny, "M_BYTES_TINY", FieldType::BYTES_TINY)
          .field(&Player::m_bytes_medium, "M_BYTES_MEDIUM", FieldType::BYTES_MEDIUM)
          .field(&Player::m_bytes_long, "M_BYTES_LONG", FieldType::BYTES_LONG)
          .index("NAME")
          .key("ID");
}
