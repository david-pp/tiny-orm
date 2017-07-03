#include "tinyreflection.h"

// User Defined Struct
struct Player {
    int id = 0;
    std::string name;
    int score = 0;
    float salary = 20000.25;
    std::vector<int> vec = {21, 22 , 23, 24, 25};
    std::vector<std::string> vec2 = {"david", "LL", "JJ"};
    std::vector<std::vector<std::string> > vec3 = {
            {"david1", "LL", "JJ"},
            {"david2", "LL", "JJ"},
            {"david3", "LL", "JJ"}
    };

    void dump() {
        std::cout << std::dec << id << "," << name << "," << score << "," << salary << std::endl;

        std::cout << "vec = [";
        std::for_each(vec.begin(), vec.end(), [](int v) {
            std::cout << v << ",";
        });
        std::cout << "]" << std::endl;

        std::cout << "vec2 = [";
        std::for_each(vec2.begin(), vec2.end(), [](std::string& v) {
            std::cout << v << ",";
        });
        std::cout << "]" << std::endl;

        std::cout << "vec3 = [";
        std::for_each(vec3.begin(), vec3.end(), [](std::vector<std::string>& v) {
            std::cout << "[";
            std::for_each(v.begin(), v.end(), [](std::string& v1) {
                std::cout << v1 << ",";
            });
            std::cout << "]," ;
        });
        std::cout << "]" << std::endl;
    }
};

// Player's Reflection
RUN_ONCE(Player){
    StructFactory::instance().declare<Player>("Player").version(2016)
            .property("id", &Player::id, 1)
            .property("name", &Player::name, 2)
            .property("score", &Player::score, 3)
            .property("salary", &Player::salary, 4)
            .property("vec", &Player::vec, 5)
            .property("vec2", &Player::vec2, 6)
            .property("vec3", &Player::vec3, 7);
}


void demo_1() {

    Player p;
    auto reflection = StructFactory::instance().structByType<Player>();

    std::cout << "\n-----------relection:set ------------\n";
    {
        reflection->set<int>(p, "id", 100);
        reflection->set<std::string>(p, "name", "david");

        std::cout << p.id << std::endl;
        std::cout << p.name << std::endl;
    }

    std::cout << "\n-----------relection:get ------------\n";
    {
        std::cout << reflection->get<int>(p, "id") << std::endl;
        std::cout << reflection->get<std::string>(p, "name") << std::endl;
    }

    std::cout << "\n-----------relection:iteration ------------\n";
    {
        for (auto prop : reflection->propertyIterator()) {
            if (prop->type() == typeid(int))
                std::cout << prop->name() << ":" << reflection->get<int>(p, prop->name()) << std::endl;
            else if (prop->type() == typeid(std::string))
                std::cout << prop->name() << ":" << reflection->get<std::string>(p, prop->name()) << std::endl;
        }
    }
}

int main() {
    demo_1();
}