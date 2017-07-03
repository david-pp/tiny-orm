#include "tinyreflection.h"
#include "tinyserializer.h"
#include "tinyserializer_proto.h"
#include "tinyserializer_proto_dyn.h"

#include "player.pb.h"

//
// Demo for Scalar
//
void demo_basic() {

    std::cout << "\n========" << __PRETTY_FUNCTION__ << std::endl;

    std::cout << "\n------ int ----------\n";
    {
        uint32_t v1 = 1024;
        std::string data = serialize<ProtoDynSerializer>(v1);

        uint64_t v2 = 0;
        deserialize<ProtoDynSerializer>(v2, data);

        std::cout << v2 << std::endl;
    }

    std::cout << "\n------ float ----------\n";
    {
        double v1 = 3.1415926;
        std::string data = serialize<ProtoDynSerializer>(v1);

        float v2 = 0;
        deserialize<ProtoDynSerializer>(v2, data);

        std::cout << v2 << std::endl;
    }

    std::cout << "\n------ string ----------\n";
    {
        std::string v1 = "Hello David++!";
        std::string data = serialize<ProtoDynSerializer>(v1);

        std::string v2;
        if (deserialize<ProtoDynSerializer>(v2, data))
            std::cout << v2 << std::endl;
        else
            std::cout << "error happens !" << std::endl;
    }

    std::cout << "\n------ proto ----------\n";
    {
        PlayerProto p1;
        p1.set_id(1024);
        p1.set_name("david");
        p1.add_quests(1);
        p1.add_quests(2);
        p1.mutable_weapon()->set_type(2);
        p1.mutable_weapon()->set_name("Sword");


        std::string data = serialize<ProtoDynSerializer>(p1);

        PlayerProto p2;
        if (deserialize<ProtoDynSerializer>(p2, data))
            std::cout << p2.ShortDebugString() << std::endl;
        else
            std::cout << "error happens !" << std::endl;
    }
}

//
// Demo for STL container
//
void demo_stl() {
    std::cout << "\n========" << __PRETTY_FUNCTION__ << std::endl;

    PlayerProto p;
    p.set_id(1024);
    p.set_name("david");
    p.add_quests(1);
    p.add_quests(2);
    p.mutable_weapon()->set_type(2);
    p.mutable_weapon()->set_name("Sword");

    std::cout << "\n------ vector<int> ----------\n";
    {
        std::vector<uint8_t> v1 = {1, 2, 3, 4, 5, 6};
        std::string data = serialize<ProtoDynSerializer>(v1);

        std::vector<uint32_t> v2;
        deserialize<ProtoDynSerializer>(v2, data);

        for (auto &v : v2)
            std::cout << v << ",";
        std::cout << std::endl;
    }

    std::cout << "\n------ map<int, PlayerProto> ----------\n";
    {
        std::map<uint32_t, PlayerProto> v1 = {
                {1024, p},
                {1025, p},
                {1026, p},
        };

        std::string data = serialize<ProtoDynSerializer>(v1);

        std::map<uint64_t, PlayerProto> v2;
        deserialize<ProtoDynSerializer>(v2, data);

        for (auto &v : v2)
            std::cout << v.first << " - " << v.second.ShortDebugString() << std::endl;
    }

    std::cout << "\n------ map<int, std::vector<PlayerProto>> ----------\n";
    {
        std::map<uint32_t, std::vector<PlayerProto>> v1 = {
                {1024, {p, p, p}},
                {1025, {p, p}},
                {1026, {p}},
        };

        std::string data = serialize<ProtoDynSerializer>(v1);

        std::map<uint32_t, std::vector<PlayerProto>> v2;
        deserialize<ProtoDynSerializer>(v2, data);

        for (auto &v : v2) {
            std::cout << v.first << std::endl;
            for (auto& player : v.second)
                std::cout << "\t - " << player.ShortDebugString() << std::endl;
        }
    }
}

//
// Demo for User-Defined
//

struct Weapon {
    uint32_t type = 0;
    std::string name = "";
};

struct Player {
    uint32_t id = 0;
    std::string name = "";
    std::vector<uint32_t> quests;
    Weapon weapon;
    std::map<uint32_t, Weapon> weapons;
    std::map<uint32_t, std::vector<Weapon>> weapons_map;

    void dump() const {
        std::cout << "id   = " << id << std::endl;
        std::cout << "name = " << name << std::endl;

        std::cout << "quests = [";
        for (auto v : quests)
            std::cout << v << ",";
        std::cout << "]" << std::endl;

        std::cout << "weapon = {" << weapon.type << "," << weapon.name << "}" << std::endl;

        std::cout << "weapons_map = {" << std::endl;
        for (auto &kv : weapons_map) {
            std::cout << "\t" << kv.first << ": [";
            for (auto &p : kv.second) {
                std::cout << "{" << p.type << "," << p.name << "},";
            }
            std::cout << "]" << std::endl;
        }
        std::cout << "}" << std::endl;
    }

    void init() {
        id = 1024;
        name = "david";
        quests.push_back(1);
        quests.push_back(2);

        weapon.type = 0;
        weapon.name = "Sword";

        for (uint32_t i = 0; i < 3; i++) {
            weapons[i].type = i;
            weapons[i].name = "Shield";
        }

        Weapon w;
        w.type = 22;
        w.name = "Blade";
        weapons_map = {
                {1, {w, w, w}},
                {2, {w, w, w}},
        };

    }
};

// Struct -> Proto Mapping
RUN_ONCE(Mapping) {

    ProtoMappingFactory::instance().declare<Weapon>("Weapon")
            .property<ProtoDynSerializer>("type", &Weapon::type, 1)
            .property<ProtoDynSerializer>("name", &Weapon::name, 2);

    ProtoMappingFactory::instance().declare<Player>("Player", "PlayerDynProto")
            .property<ProtoDynSerializer>("id", &Player::id, 1)
            .property<ProtoDynSerializer>("name", &Player::name, 2)
            .property<ProtoDynSerializer>("quests", &Player::quests, 3)
            .property<ProtoDynSerializer>("weapon", &Player::weapon, 4)
            .property<ProtoDynSerializer>("weapon2", &Player::weapons, 5)
            .property<ProtoDynSerializer>("weapons_map", &Player::weapons_map, 6);

    // MUST!!! CREATE DESCRIPTORS
    ProtoMappingFactory::instance().createAllProtoDescriptor();
}

void demo_userdefined() {
    std::cout << "\n========" << __PRETTY_FUNCTION__ << std::endl;

    try {
        std::cout << "\n------ Proto created ----------\n";
        {
            ProtoMappingFactory::instance().createAllProtoDefine();
        }

        std::cout << "\n------ Weapon ----------\n";
        {
            Weapon w;
            w.type = 22;
            w.name = "Blade";

            std::string data = serialize<ProtoDynSerializer>(w);

            Weapon w2;
            deserialize<ProtoDynSerializer>(w2, data);
            std::cout << w2.type << " - " << w2.name << std::endl;
        }

        std::cout << "\n------ Player ----------\n";
        {
            Player p;
            p.init();

            std::string data = serialize<ProtoDynSerializer>(p);

            Player p2;
            deserialize<ProtoDynSerializer>(p2, data);
            p2.dump();
        }
    } catch (const std::exception& e) {
        std::cout << "Error Happens: " << e.what() << std::endl;
    }
}

//
// Demo for Archiver
//
void demo_archiver() {
    std::cout << "\n========" << __PRETTY_FUNCTION__ << std::endl;

    std::string data;

    // serialize
    {
        PlayerProto p;
        p.set_id(1024);
        p.set_name("david");

        std::vector<PlayerProto> pvec{p, p, p};

        std::map<uint32_t, PlayerProto> pmap{
                {1, p},
                {2, p},
                {3, p}
        };

        std::vector<std::map<uint32_t, PlayerProto> > vecmap{pmap, pmap, pmap};

        ProtoArchiver<ProtoDynSerializer> ar;
        ar << p << p << p;        // support: protobuf generated
        ar << pvec;               // support: std::vector
        ar << pmap;               // support: std::map
        ar << vecmap;             // support: composite of STL container
        ar << ar;                 // support: archiver self ? YES!

//        ar.SerializeToString(&data);
        data = serialize(ar);
    }

    // deserialize
    {
        PlayerProto p1, p2, p3;
        std::vector<PlayerProto> pvec;
        std::map<uint32_t, PlayerProto> pmap;
        std::vector<std::map<uint32_t, PlayerProto> > vecmap;

        ProtoArchiver<ProtoDynSerializer> ar, ar2;
//        if (ar.ParseFromString(data)) {
        if (::deserialize(ar, data)) {
            ar >> p1 >> p2 >> p3;
            ar >> pvec;
            ar >> pmap;
            ar >> vecmap;
            ar >> ar2;
        }

        std::cout << "\n------ p1 ----------\n";
        {
            std::cout << p1.ShortDebugString() << std::endl;
        }
        std::cout << "\n------ p2 ----------\n";
        {
            std::cout << p2.ShortDebugString() << std::endl;
        }
        std::cout << "\n------ p3 ----------\n";
        {
            std::cout << p3.ShortDebugString() << std::endl;
        }

        std::cout << "\n------ vector<P> ----------\n";
        {
            for (auto &p : pvec) {
                std::cout << p.ShortDebugString() << std::endl;
            }
        }

        std::cout << "\n------ map<P> ----------\n";
        {
            for (auto &v : pmap) {
                std::cout << v.first << " : " << v.second.ShortDebugString() << std::endl;
            }
        }

        std::cout << "\n------ vector<map<P>> ----------\n";
        {
            for (auto &item : vecmap) {
                for (auto &v: item)
                    std::cout << v.first << " : " << v.second.ShortDebugString() << std::endl;
                std::cout << std::endl;
            }
        }

        std::cout << "\n------ archiver ----------\n";
        {
            std::cout << ar2.ShortDebugString() << std::endl;
        }
    }
}

int main(int argc, const char* argv[]) {
    int count = 1;
    if (argc > 1)
        count = std::atoi(argv[1]);

    for (int i = 0; i < count; ++i) {
        demo_basic();
        demo_stl();
        demo_userdefined();
        demo_archiver();
    }
}