#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include "catch.hpp"

#include <algorithm>
#include <iostream>

#include "tinyreflection.h"
#include "tinyserializer.h"
#include "tinyserializer_proto.h"
#include "tinyserializer_proto_dyn.h"

#include "../example/player.pb.h"

template<typename T1, typename T2 = T1>
bool check_scalar(const T1 &value) {
    T1 v1 = value;
    std::string data = serialize(v1);

    T2 v2 = T2();
    deserialize(v2, data);

    return v1 == static_cast<T1>(v2);
}

TEST_CASE("serialize scalar types", "[ProtoSerializer]") {

    SECTION("integer") {
        REQUIRE(check_scalar<uint8_t>(255));
        REQUIRE(check_scalar<uint16_t>(6555));
        REQUIRE(check_scalar<uint32_t>(10244));
        REQUIRE(check_scalar<uint64_t>(102444));

        REQUIRE(check_scalar<int8_t>(255));
        REQUIRE(check_scalar<int16_t>(6555));
        REQUIRE(check_scalar<int32_t>(10244));
        REQUIRE(check_scalar<int64_t>(102444));

        REQUIRE(check_scalar<bool>(false));

        REQUIRE((check_scalar<uint32_t, int32_t>(10244)));
        REQUIRE((check_scalar<uint32_t, uint64_t>(10244)));
        REQUIRE((check_scalar<bool, int32_t>(true)));
    }

    SECTION("float") {
        REQUIRE(check_scalar<float>(3.1415));
        REQUIRE(check_scalar<double>(3.1415));
        REQUIRE((check_scalar<float, double>(3.1415)));
    }

    SECTION("string") {
        REQUIRE(check_scalar<std::string>("david++"));
    }
}

TEST_CASE("serialize proto types", "[ProtoSerializer]") {

    PlayerProto p1;
    p1.set_id(1024);
    p1.set_name("david");
    p1.add_quests(1);
    p1.add_quests(2);
    p1.mutable_weapon()->set_type(2);
    p1.mutable_weapon()->set_name("Sword");

    std::string data = serialize(p1);

    PlayerProto p2;
    deserialize(p2, data);

    CHECK(p1.id() == p2.id());
    CHECK(p1.name() == p2.name());
    CHECK(p1.quests_size() == p2.quests_size());
    CHECK(p1.weapon().type() == p1.weapon().type());
    CHECK(p1.weapon().name() == p1.weapon().name());
}


template<typename T1, typename T2 = T1>
bool check_sequence() {
    T1 v1 = {1, 2, 3, 4, 5, 6, 7, 8};
    std::string data = serialize(v1);

    T2 v2 = T2();
    deserialize(v2, data);

    if (v1.size() != v2.size())
        return false;

    return std::equal(v1.begin(), v1.end(), v2.begin());
}

template<typename T1, typename T2 = T1>
bool check_set() {
    T1 v1 = {1, 2, 3, 4, 5, 6, 7, 8};
    std::string data = serialize(v1);

    T2 v2 = T2();
    deserialize(v2, data);

    if (v1.size() != v2.size())
        return false;

    for (auto &v : v1)
        if (v2.find(v) == v2.end())
            return false;

    return true;
}

template<typename T1, typename T2 = T1>
bool check_map() {

    PlayerProto p;
    p.set_id(1024);
    p.set_name("david");
    p.add_quests(1);
    p.add_quests(2);
    p.mutable_weapon()->set_type(2);
    p.mutable_weapon()->set_name("Sword");

    T1 v1 = {
            {1, p},
            {2, p},
            {3, p},
    };

    std::string data = serialize(v1);

    T2 v2 = T2();
    deserialize(v2, data);

    if (v1.size() != v2.size())
        return false;

    for (auto &v : v1) {
        auto it2 = v2.find(v.first);
        if (it2 == v2.end())
            return false;

        if (it2->second.id() != v.second.id())
            return false;

        if (it2->second.name() != v.second.name())
            return false;

        if (it2->second.quests_size() != v.second.quests_size())
            return false;
    }

    return true;
}

TEST_CASE("serialize STL containers", "[ProtoSerializer]") {

    SECTION("Sequence Container") {
        CHECK(check_sequence<std::vector<uint32_t>>());
        CHECK(check_sequence<std::list<uint32_t>>());
        CHECK(check_sequence<std::deque<uint32_t>>());

        CHECK((check_sequence<std::vector<uint32_t>, std::list<uint64_t>>()));
    }

    SECTION("Set Container") {
        CHECK(check_set<std::set<uint32_t>>());
        CHECK(check_set<std::multiset<uint32_t>>());
        CHECK(check_set<std::unordered_set<uint32_t>>());
        CHECK(check_set<std::unordered_multiset<uint32_t>>());

        CHECK((check_set<std::set<uint32_t>, std::multiset<uint32_t>>()));
        CHECK((check_set<std::unordered_set<uint32_t>, std::unordered_multiset<uint32_t>>()));
        CHECK((check_set<std::set<uint32_t>, std::unordered_set<uint32_t>>()));
        CHECK((check_set<std::set<uint32_t>, std::unordered_multiset<uint32_t>>()));
    }

    SECTION("Map Container") {
        CHECK((check_map<std::map<uint32_t, PlayerProto>>()));
        CHECK((check_map<std::multimap<uint32_t, PlayerProto>>()));
        CHECK((check_map<std::unordered_map<uint32_t, PlayerProto>>()));
        CHECK((check_map<std::unordered_multimap<uint32_t, PlayerProto>>()));

        CHECK((check_map<std::map<uint32_t, PlayerProto>, std::multimap<uint16_t, PlayerProto>>()));
        CHECK((check_map<std::unordered_map<uint32_t, PlayerProto>, std::unordered_multimap<uint16_t, PlayerProto>>()));
        CHECK((check_map<std::map<uint32_t, PlayerProto>, std::unordered_map<uint16_t, PlayerProto>>()));
    }
}

struct Weapon {
    uint32_t type = 0;
    std::string name = "";

    //
    // intrusive way
    //
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

TEST_CASE("serialize user defined struct", "[ProtoSerializer]") {

    Weapon w;
    w.type = 22;
    w.name = "Blade";

    std::string data = serialize(w);

    Weapon w2;
    deserialize(w2, data);

    CHECK(w.type == w2.type);
    CHECK(w.name == w2.name);
}


TEST_CASE("serialize<ProtoDynSerializer> defined struct", "[ProtoDynSerializer]") {

    ProtoMappingFactory::instance().declare<Weapon>("Weapon")
            .property<ProtoDynSerializer>("type", &Weapon::type, 1)
            .property<ProtoDynSerializer>("name", &Weapon::name, 2)
            .done();


    std::cout << ProtoMappingFactory::instance().protoDefineByType<Weapon>() << std::endl;

    Weapon w;
    w.type = 22;
    w.name = "Blade";

    std::string data = serialize<ProtoDynSerializer>(w);

    Weapon w2;
    deserialize<ProtoDynSerializer>(w2, data);

    CHECK(w.type == w2.type);
    CHECK(w.name == w2.name);
}
