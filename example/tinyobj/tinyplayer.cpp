#include "tinyplayer.h"
#include "tinyplayer.pb.h"

namespace tiny {

std::string Friend::serialize() const {
     FriendProto proto;
     proto.set_id(id);
     proto.set_name(name);
     return proto.SerializeAsString();
}


bool Friend::deserialize(const std::string &data) {
     FriendProto proto;
     if (!proto.ParseFromString(data)) return false;
     id = proto.id();
     name = proto.name();
     return true;
}


std::string Country::serialize() const {
     CountryProto proto;
     proto.set_id(id);
     proto.set_name(name);
     return proto.SerializeAsString();
}


bool Country::deserialize(const std::string &data) {
     CountryProto proto;
     if (!proto.ParseFromString(data)) return false;
     id = proto.id();
     name = proto.name();
     return true;
}


std::string Player::serialize() const {
     PlayerProto proto;
     proto.set_id(id);
     proto.set_name(name);
     proto.set_country(::serialize(country));
     proto.set_friends(::serialize(friends));
     proto.set_m_int8(m_int8);
     proto.set_m_int16(m_int16);
     proto.set_m_int32(m_int32);
     proto.set_m_int64(m_int64);
     proto.set_m_uint8(m_uint8);
     proto.set_m_uint16(m_uint16);
     proto.set_m_uint32(m_uint32);
     proto.set_m_uint64(m_uint64);
     proto.set_m_float(m_float);
     proto.set_m_double(m_double);
     proto.set_m_bool(m_bool);
     proto.set_m_string(m_string);
     proto.set_m_bytes(m_bytes);
     proto.set_m_bytes_tiny(m_bytes_tiny);
     proto.set_m_bytes_medium(m_bytes_medium);
     proto.set_m_bytes_long(m_bytes_long);
     return proto.SerializeAsString();
}


bool Player::deserialize(const std::string &data) {
     PlayerProto proto;
     if (!proto.ParseFromString(data)) return false;
     id = proto.id();
     name = proto.name();
     ::deserialize(country, proto.country());
     ::deserialize(friends, proto.friends());
     m_int8 = proto.m_int8();
     m_int16 = proto.m_int16();
     m_int32 = proto.m_int32();
     m_int64 = proto.m_int64();
     m_uint8 = proto.m_uint8();
     m_uint16 = proto.m_uint16();
     m_uint32 = proto.m_uint32();
     m_uint64 = proto.m_uint64();
     m_float = proto.m_float();
     m_double = proto.m_double();
     m_bool = proto.m_bool();
     m_string = proto.m_string();
     m_bytes = proto.m_bytes();
     m_bytes_tiny = proto.m_bytes_tiny();
     m_bytes_medium = proto.m_bytes_medium();
     m_bytes_long = proto.m_bytes_long();
     return true;
}

} // namespace tiny
