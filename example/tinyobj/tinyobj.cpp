#include <iostream>
#include <vector>
#include "tinylogger.h"
#include "tinyplayer.h"

void upateTables()
{
    TinyMySqlORM orm;
    orm.dropTableByName("PLAYER");
    orm.updateTables();
}

void test_1()
{
    tiny::Player p;
    p.id = 1024;
    p.country.name = "中国";
    p.country.id = 2;
    p.friends.resize(2);
    p.friends[0].id = 1;
    p.friends[0].name = "david";
    p.friends[1].id = 2;
    p.friends[1].name = "lica";

    TinyMySqlORM orm;
    orm.insert(p);
}

void test_2()
{
    tiny::Player p;
    p.id = 1024;

    TinyMySqlORM mysql;
    mysql.select(p);

    std::cout << p.friends.size() << std::endl;
}

int main(int argc, const char *argv[]) {
    MySqlConnectionPool::instance().connect("mysql://david:123456@127.0.0.1/tinyworld?maxconn=5");
    upateTables();
    test_1();
    test_2();
}