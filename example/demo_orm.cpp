//ODB: C++ Object-Relational Mapping (ORM)

#include <iostream>
#include <unordered_map>
#include <vector>
#include <memory>
#include <tinyreflection.h>

#include "tinylogger.h"

#include "tinyorm.h"

#ifdef USE_ORM_MYSQLPP
# include "tinyorm_mysql.h"
#else
# include "tinyorm_soci.h"
#endif

#include "player.h"

void test_create() {
    TinyORM db;
    db.createTableByName("PLAYER");
}

void test_drop() {
    TinyORM db;
    db.dropTableByName("PLAYER");
}

void test_update() {
    TinyORM db;
    db.updateTables();
}

void test_sql() {
    TinyORM db;

#ifdef USE_ORM_MYSQLPP
    {
        mysqlpp::Query query(NULL);
        db.makeSelectQuery(query, Player());
        LOG_INFO(__FUNCTION__, "%s", query.str().c_str());
    }

    {
        mysqlpp::Query query(NULL);
        db.makeInsertQuery(query, Player());
        LOG_INFO(__FUNCTION__, "%s", query.str().c_str());
    }

    {
        mysqlpp::Query query(NULL);
        db.makeReplaceQuery(query, Player());
        LOG_INFO(__FUNCTION__, "%s", query.str().c_str());
    }

    {
        mysqlpp::Query query(NULL);
        db.makeUpdateQuery(query, Player());
        LOG_INFO(__FUNCTION__, "%s", query.str().c_str());
    }

    {
        mysqlpp::Query query(NULL);
        db.makeDeleteQuery(query, Player());
        LOG_INFO(__FUNCTION__, "%s", query.str().c_str());
    }
#endif
}

void test_insertDB() {

    TinyORM db;
    for (uint32_t i = 0; i < 10; ++i) {
        Player p;
        p.init();
        p.id = i;
        p.age = 30;
        p.name = "david-insert-" + std::to_string(i);
        db.insert(p);
    }
};

void test_replaceDB() {
    TinyORM db;
    for (uint32_t i = 0; i < 10; ++i) {
        Player p;
        p.init();
        p.id = i;
        p.age = 30;
        p.name = "david-replace-" + std::to_string(i);
        db.replace(p);
    }
}

void test_updateDB() {
    TinyORM db;
    for (uint32_t i = 0; i < 10; ++i) {
        Player p;
        p.init();
        p.id = i;
        p.age = 30;
        p.name = "david-update-" + std::to_string(i);
        db.update(p);
    }
}

void test_deleteDB() {
    TinyORM db;
    for (uint32_t i = 0; i < 10; ++i) {
        Player p;
        p.id = i;
        db.del(p);
    }
}

void test_selectDB() {
    TinyORM db;
    for (uint32_t i = 0; i < 10; ++i) {
        Player p;
        p.id = i;
        p.age = 0;
        p.name = "";
        if (db.select(p))
            std::cout << p << std::endl;
    }
}

void test_load() {
    TinyORM db;
    TinyORM::Records <Player> players;
    db.loadFromDB(players, "WHERE ID %% %d=0 ORDER BY ID DESC", 2);

    for (auto p : players) {
        std::cout << *p;
    }
}

void test_load2() {
    TinyORM db;
    db.loadFromDB<Player>([](std::shared_ptr<Player> p) {
        std::cout << *p;
    }, nullptr);
}

void test_load3() {
    TinyORM db;

    struct PlayerCompare {
        bool operator()(const std::shared_ptr<Player> &lhs, const std::shared_ptr<Player> &rhs) const {
            return lhs->id > rhs->id;
        }
    };

//    using PlayerSet = std::set<std::shared_ptr<Player>, PlayerCompare>;
    using PlayerSet = std::set<std::shared_ptr<Player>>;

    PlayerSet players;
    db.loadFromDB<Player, PlayerSet>(players, nullptr);

    for (auto it = players.begin(); it != players.end(); ++it) {
        std::cout << *(*it);
    }
}


#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>

namespace tiny {
    using boost::multi_index_container;
    using namespace boost::multi_index;

    struct by_id {
    };
    struct by_name {
    };

    using PlayerSet = boost::multi_index_container<
            Player,
            indexed_by<
                    // sort by employee::operator<
                    ordered_unique<tag<by_id>, member<Player, uint32_t, &Player::id> >,
                    // sort by less<string> on name
                    ordered_non_unique<tag<by_name>, member<Player, std::string, &Player::name> >
            >
    >;
}

void test_load4() {
    TinyORM db;

    tiny::PlayerSet players;
    db.loadFromDB2MultiIndexSet<Player, tiny::PlayerSet>(players, nullptr);

    auto &players_by_id = players.get<tiny::by_id>();
    auto &players_by_name = players.get<tiny::by_name>();

    auto it = players_by_id.find(5);
    if (it != players_by_id.end()) {
        std::cout << (*it);
    }

    for (auto it = players_by_name.begin(); it != players_by_name.end(); ++it) {
        std::cout << (*it);
    }

    std::cout << players_by_name.count("david") << std::endl;
}


void test_delete() {
    TinyORM db;
    db.deleteFromDB<Player>("WHERE ID %% %d=0", 2);
}

void test_obj2db() {
    Object2DB<Player> p2db;
    p2db.id = 1024;
    p2db.name = "david-p2db";

    p2db.dropTable();
    p2db.updateTable();

    p2db.name = "david-insert";
    p2db.insertDB();
    if (p2db.selectDB())
        std::cout << p2db.name << std::endl;

    p2db.name = "david-update";
    p2db.updateDB();
    if (p2db.selectDB())
        std::cout << p2db.name << std::endl;

    p2db.name = "david-replace";
    p2db.replaceDB();
    if (p2db.selectDB())
        std::cout << p2db.name << std::endl;

    p2db.deleteDB();
    if (p2db.selectDB())
        std::cout << p2db.name << std::endl;


//    p2db.id = 8;
//    p2db.selectDB(&MySqlConnectionPool::instance());
//    std::cout << p2db;
//
//    ScopedMySqlConnection mysql;
//    if (mysql) {
//        p2db.id = 7;
//        p2db.selectDB(mysql.get());
//        std::cout << p2db;
//    }
}

int main(int argc, const char *argv[]) {
    if (argc < 2) {
        std::cout << "Usage:" << argv[0]
                  << " create | drop | update" << std::endl;
        return 1;
    }

#ifdef USE_ORM_MYSQLPP
    MySqlConnectionPool::instance().connect("mysql://david:123456@127.0.0.1/tinyworld?maxconn=5");
#else
    SOCIPool::instance().connect("mysql://host=127.0.0.1 db=tinyworld user=david password='123456'", 5);
#endif

    std::string op = argv[1];
    if ("sql" == op)
        test_sql();
    else if ("create" == op)
        test_create();
    else if ("drop" == op)
        test_drop();
    else if ("update" == op)
        test_update();
    else if ("insertDB" == op)
        test_insertDB();
    else if ("replaceDB" == op)
        test_replaceDB();
    else if ("updateDB" == op)
        test_updateDB();
    else if ("deleteDB" == op)
        test_deleteDB();
    else if ("selectDB" == op)
        test_selectDB();
    else if ("load" == op)
        test_load();
    else if ("load2" == op)
        test_load2();
    else if ("load3" == op)
        test_load3();
    else if ("load4" == op)
        test_load4();
    else if ("delete" == op)
        test_delete();
    else if ("obj2db" == op)
        test_obj2db();

    return 0;
}

