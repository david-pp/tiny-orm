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

//
// Impliment the Serializer Concept by Protobuf
//

#ifndef TINYWORLD_TINYSERIALIZER_PROTO_H
#define TINYWORLD_TINYSERIALIZER_PROTO_H

#include <string>
#include <vector>
#include <list>
#include <deque>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>

#include "tinyworld.h"
#include "archive.pb.h"

//
// T -> int
//
template<typename T>
struct ProtoCase;

template<typename T, uint32_t proto_case>
struct ProtoSerializerImpl;

//
// Proto Serializer
//
template<typename T>
struct ProtoSerializer : public ProtoSerializerImpl<T, ProtoCase<T>::value> {
};

//
// Archiver : serialize and deserialize by the same order
//
template <template <typename> class SerializerT = ProtoSerializer>
class ProtoArchiver : public ArchiveProto {
public:
    template<typename T>
    ProtoArchiver &operator<<(const T &object) {
        ArchiveMemberProto *mem = this->add_members();
        if (mem) {
            mem->set_data(serialize<SerializerT>(object));
        }
        return *this;
    }

    template<typename T>
    ProtoArchiver &operator>>(T &object) {
        if (read_pos_ < this->members_size()) {
            const ArchiveMemberProto &mem = this->members(read_pos_);
            deserialize<SerializerT>(object, mem.data());
            read_pos_++;
        }
        return *this;
    }

private:
    int read_pos_ = 0;
};


//
// Proto Case
//
enum ProtoTypeEnum {
    kProtoType_UserDefined,
    kProtoType_Proto,
    kProtoType_Integer,
    kProtoType_Float,
    kProtoType_String,
    kProtoType_Seq,
    kProtoType_Set,
    kProtoType_Map,
    kProtoType_HashSet,
    kProtoType_HashMap,
};

template<typename T>
struct ProtoCase : public std::integral_constant<ProtoTypeEnum,
        std::is_base_of<google::protobuf::Message, T>::value ? kProtoType_Proto : kProtoType_UserDefined> {
};

// Scalar Type Case
#define PROTO_CASE_SCALAR(ScalarType, TypeEnum) \
    template<> struct ProtoCase<ScalarType> : public std::integral_constant<ProtoTypeEnum, TypeEnum> {}

// List Type Case
#define PROTO_CASE_SEQ(SeqType) \
    template<typename T, typename AllocT> struct ProtoCase<SeqType<T, AllocT> > \
                 : public std::integral_constant<ProtoTypeEnum, kProtoType_Seq> {}
// Set Type Case
#define PROTO_CASE_SET(SetType) \
    template<typename KeyT, typename CompareT, typename AllocT> struct ProtoCase<SetType<KeyT, CompareT, AllocT> > \
                 : public std::integral_constant<ProtoTypeEnum, kProtoType_Set> {}
#define PROTO_CASE_HASHSET(SetType) \
    template<typename Key, typename Hash, typename KeyEqual, typename Allocator> \
    struct ProtoCase<SetType<Key, Hash, KeyEqual, Allocator> > \
                 : public std::integral_constant<ProtoTypeEnum, kProtoType_HashSet> {}

// Map Type Case
#define PROTO_CASE_MAP(MapType) \
    template<typename KeyT, typename ValueT, typename AllocT> struct ProtoCase<MapType<KeyT, ValueT, AllocT> >  \
                 : public std::integral_constant<ProtoTypeEnum, kProtoType_Map> {}
#define PROTO_CASE_HASHMAP(MapType) \
    template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator> \
    struct ProtoCase<MapType<Key, T, Hash, KeyEqual, Allocator> > \
                 : public std::integral_constant<ProtoTypeEnum, kProtoType_HashMap> {}

// Integer
PROTO_CASE_SCALAR(int8_t, kProtoType_Integer);
PROTO_CASE_SCALAR(int16_t, kProtoType_Integer);
PROTO_CASE_SCALAR(int32_t, kProtoType_Integer);
PROTO_CASE_SCALAR(int64_t, kProtoType_Integer);

PROTO_CASE_SCALAR(uint8_t, kProtoType_Integer);
PROTO_CASE_SCALAR(uint16_t, kProtoType_Integer);
PROTO_CASE_SCALAR(uint32_t, kProtoType_Integer);
PROTO_CASE_SCALAR(uint64_t, kProtoType_Integer);

// Boolean
PROTO_CASE_SCALAR(bool, kProtoType_Integer);

// Float
PROTO_CASE_SCALAR(float, kProtoType_Float);
PROTO_CASE_SCALAR(double, kProtoType_Float);

// String
PROTO_CASE_SCALAR(std::string, kProtoType_String);

// Sequence Container:
PROTO_CASE_SEQ(std::vector);
PROTO_CASE_SEQ(std::list);
PROTO_CASE_SEQ(std::deque);

// Set
PROTO_CASE_SET(std::set);
PROTO_CASE_SET(std::multiset);
PROTO_CASE_HASHSET(std::unordered_set);
PROTO_CASE_HASHSET(std::unordered_multiset);

// Map
PROTO_CASE_MAP(std::map);
PROTO_CASE_MAP(std::multimap);
PROTO_CASE_HASHMAP(std::unordered_map);
PROTO_CASE_HASHMAP(std::unordered_multimap);


//
// Impl ====================================================
//

//
// Interger
//
template<typename T>
struct ProtoSerializerImpl<T, kProtoType_Integer> {

    std::string serialize(const T &value) const {
        IntegerProto proto;
        proto.set_value(value);
        return proto.SerializeAsString();
    }

    bool deserialize(T &value, const std::string &data) const {
        IntegerProto proto;
        if (proto.ParseFromString(data)) {
            value = proto.value();
            return true;
        }
        return false;
    }
};


//
// Float
//
template<typename T>
struct ProtoSerializerImpl<T, kProtoType_Float> {

    std::string serialize(const T &value) const {
        FloatProto proto;
        proto.set_value(value);
        return proto.SerializeAsString();
    }

    bool deserialize(T &value, const std::string &data) const {
        FloatProto proto;
        if (proto.ParseFromString(data)) {
            value = proto.value();
            return true;
        }
        return false;
    }
};

//
// String
//
template<typename T>
struct ProtoSerializerImpl<T, kProtoType_String> {

    std::string serialize(const T &value) const {
        StringProto proto;
        proto.set_value(value);
        return proto.SerializeAsString();
    }

    bool deserialize(T &value, const std::string &data) const {
        StringProto proto;
        if (proto.ParseFromString(data)) {
            value = proto.value();
            return true;
        }
        return false;
    }
};


//
// Generated by Protobuf
//
template<typename T>
struct ProtoSerializerImpl<T, kProtoType_Proto> {
public:
    std::string serialize(const T &proto) const {
        return proto.SerializeAsString();
    }

    bool deserialize(T &proto, const std::string &data) const {
        return proto.ParseFromString(data);
    }
};

//
// Sequence Container: vector, list, deque
//
template<typename T, typename Allocator, template<typename, typename> class Container>
struct ProtoSerializerImpl<Container<T, Allocator>, kProtoType_Seq> {

    std::string serialize(const Container<T, Allocator> &objects) const {
        SequenceProto proto;
        ProtoSerializer<T> member_serializer;
        for (auto &v : objects) {
            ArchiveMemberProto *mem = proto.add_values();
            if (mem) {
                mem->set_data(member_serializer.serialize(v));
            }
        }

        return proto.SerializeAsString();
    }

    bool deserialize(Container<T, Allocator> &objects, const std::string &data) const {
        SequenceProto proto;
        ProtoSerializer<T> member_serializer;
        if (proto.ParseFromString(data)) {
            for (int i = 0; i < proto.values_size(); ++i) {
                T obj;
                if (member_serializer.deserialize(obj, proto.values(i).data()))
                    objects.push_back(obj);
            }
            return true;
        }
        return false;
    }
};

//
// Set Container: set, multiset
//
template<typename Key, typename Compare, typename Allocator, template<typename, typename, typename> class Set>
struct ProtoSerializerImpl<Set<Key, Compare, Allocator>, kProtoType_Set> {

    typedef Set<Key, Compare, Allocator> SetType;

    std::string serialize(const SetType &objects) const {
        SequenceProto proto;
        ProtoSerializer<Key> member_serializer;
        for (auto &v : objects) {
            ArchiveMemberProto *mem = proto.add_values();
            if (mem) {
                mem->set_data(member_serializer.serialize(v));
            }
        }

        return proto.SerializeAsString();
    }

    bool deserialize(SetType &objects, const std::string &data) const {
        SequenceProto proto;
        ProtoSerializer<Key> member_serializer;
        if (proto.ParseFromString(data)) {
            for (int i = 0; i < proto.values_size(); ++i) {
                Key obj;
                if (member_serializer.deserialize(obj, proto.values(i).data()))
                    objects.insert(obj);
            }
            return true;
        }
        return false;
    }
};

//
// Map Container: map, multimap
//
template<typename Key, typename T, typename Compare, typename Allocator, template<typename, typename, typename, typename> class Map>
struct ProtoSerializerImpl<Map<Key, T, Compare, Allocator>, kProtoType_Map> {
    typedef Map<Key, T, Compare, Allocator> MapType;

    std::string serialize(const MapType &objects) const {
        AssociateProto proto;
        ProtoSerializer<Key> key_serializer;
        ProtoSerializer<T> value_serializer;
        for (auto &v : objects) {
            AssociateProto::ValueType *mem = proto.add_values();
            if (mem) {
                ArchiveMemberProto *key = mem->mutable_key();
                ArchiveMemberProto *value = mem->mutable_value();
                if (key && value) {
                    key->set_data(key_serializer.serialize(v.first));
                    value->set_data(value_serializer.serialize(v.second));
                }
            }
        }

        return proto.SerializeAsString();
    }

    bool deserialize(MapType &objects, const std::string &bin) const {
        AssociateProto proto;
        ProtoSerializer<Key> key_serializer;
        ProtoSerializer<T> value_serializer;
        if (proto.ParseFromString(bin)) {
            for (int i = 0; i < proto.values_size(); ++i) {
                Key key;
                T value;
                if (key_serializer.deserialize(key, proto.values(i).key().data())
                    && value_serializer.deserialize(value, proto.values(i).value().data())) {
                    objects.insert(std::make_pair(key, value));
                }
            }
            return true;
        }
        return false;
    }
};


//
// Hash Set Container: unordered_set, unordered_multiset
//
template<typename Key, typename Hash, typename KeyEqual, typename Allocator, template<typename, typename, typename, typename> class Set>
struct ProtoSerializerImpl<Set<Key, Hash, KeyEqual, Allocator>, kProtoType_HashSet> {

    typedef Set<Key, Hash, KeyEqual, Allocator> SetType;

    std::string serialize(const SetType &objects) const {
        SequenceProto proto;
        ProtoSerializer<Key> member_serializer;
        for (auto &v : objects) {
            ArchiveMemberProto *mem = proto.add_values();
            if (mem) {
                mem->set_data(member_serializer.serialize(v));
            }
        }

        return proto.SerializeAsString();
    }

    bool deserialize(SetType &objects, const std::string &data) const {
        SequenceProto proto;
        ProtoSerializer<Key> member_serializer;
        if (proto.ParseFromString(data)) {
            for (int i = 0; i < proto.values_size(); ++i) {
                Key obj;
                if (member_serializer.deserialize(obj, proto.values(i).data()))
                    objects.insert(obj);
            }
            return true;
        }
        return false;
    }
};


//
// Hash Map Container: unordered_map, unordered_multimap
//
template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator,
        template<typename, typename, typename, typename, typename> class Map>
struct ProtoSerializerImpl<Map<Key, T, Hash, KeyEqual, Allocator>, kProtoType_HashMap> {
    typedef Map<Key, T, Hash, KeyEqual, Allocator> MapType;

    std::string serialize(const MapType &objects) const {
        AssociateProto proto;
        ProtoSerializer<Key> key_serializer;
        ProtoSerializer<T> value_serializer;
        for (auto &v : objects) {
            AssociateProto::ValueType *mem = proto.add_values();
            if (mem) {
                ArchiveMemberProto *key = mem->mutable_key();
                ArchiveMemberProto *value = mem->mutable_value();
                if (key && value) {
                    key->set_data(key_serializer.serialize(v.first));
                    value->set_data(value_serializer.serialize(v.second));
                }
            }
        }

        return proto.SerializeAsString();
    }

    bool deserialize(MapType &objects, const std::string &bin) const {
        AssociateProto proto;
        ProtoSerializer<Key> key_serializer;
        ProtoSerializer<T> value_serializer;
        if (proto.ParseFromString(bin)) {
            for (int i = 0; i < proto.values_size(); ++i) {
                Key key;
                T value;
                if (key_serializer.deserialize(key, proto.values(i).key().data())
                    && value_serializer.deserialize(value, proto.values(i).value().data())) {
                    objects.insert(std::make_pair(key, value));
                }
            }
            return true;
        }
        return false;
    }
};

//
// User Defined
//
template<typename T>
struct ProtoSerializerImpl<T, kProtoType_UserDefined> {
public:
    std::string serialize(const T &object) const {
        return object.serialize();
    }

    bool deserialize(T &object, const std::string &data) const {
        return object.deserialize(data);
    }
};

#endif //TINYWORLD_TINYSERIALIZER_PROTO_H
