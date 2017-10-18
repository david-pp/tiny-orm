TinyORM
--------------------

对象关系映射（ORM，Object-Relational Mappings），是一种程序技术，用于实现面向对象编程语言里不同类型系统的数据之间的转换。说白了，就是将C++对象映射到数据库表的一种技术，目的就是简化对象存档和交换的代码。

一般情况下，对象要存储到关系型数据库，需要在代码里面使用SQL语句，若对象较为复杂的话，就需要编写大量代码去支持对象的数据库操作，代码也不怎么美观，扩展性也比较差。ORM则封装这些操作作为对象的成员函数，这样使用时候就会比较方便。（PS. ORM会损失一定的灵活性，偶尔SQL也是必要的）


该项目尝试使用C++写一个简单的ORM，支持：
 - 一个C++结构体映射射到一张关系表。
 - 映射关系可以手工编写也可以使用工具自动生成。
 - 结构体成员变量支持多种类型，简单类型、复合类型、STL容器都有支持。
 - 复合成员或STL成员映射为表的二进制字段，此时需进行序列化，当然序列化也可以自动生成。
 - 数据库表的自动创建和更新（现阶段只支持数据库添加字段）。

不支持复杂的多表操作，数据分表等，用于存档的话完全够用了。
 
依赖：
 
 - `C++11`
 - `protobuf`
 - `mysql/mysql++` 或 `soci`
 
框架和简单用法：
 
  ![TinyORM](./docs/tinyorm.png)

## 用法

现在要定义一个游戏玩家对象，有名字、ID、年龄，同时可以装备多件武器。

第一步：定义支持映射的结构体

```c++
// 武器数据
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

// 玩家数据
struct Player {
    uint32_t id = 0;
    std::string name;
    uint8_t age = 0;
    std::vector<Weapon> weapons;
}
```

第二步：定义映射关系

把`Player`结构体映射到`PLAYER`表：

```
> DESC PLAYER;
+----------------+----------------------+------+-----+---------+-------+
| Field          | Type                 | Null | Key | Default | Extra |
+----------------+----------------------+------+-----+---------+-------+
| ID             | int(10) unsigned     | NO   | PRI | 0       |       |
| NAME           | text                 | YES  |     | NULL    |       |
| AGE            | tinyint(3)           | NO   |     | 30      |       |
| WEAPONS        | blob                 | YES  |     | NULL    |       |
+----------------+----------------------+------+-----+---------+-------+
```

```c++
TableFactory::instance().table<Player>("PLAYER")
        .field(&Player::id, "ID", FieldType::UINT32)
        .field(&Player::name, "NAME", FieldType::STRING)
        .field(&Player::age, "AGE", FieldType::UINT8, "30")
        .field(&Player::weapons, "WEAPONS", FieldType::OBJECT)
        .key("ID")
        .index("NAME");

```

第三步：直接使用

假设要把所有`PLAYER`加载到内存：

```c++
TinyORM db;
TinyORM::Records<Player> players;
db.loadFromDB(players);
```

假设要更新`PLAYER`表里面的某条记录：

```c++
Object2DB<Player> p;
p.id = 1024;
p.age = 30;
p.name = "David";
p.weapons.clear();
p.update();
```

例子详细内容，请查看：[demo_orm.cpp](./example/demo_orm.cpp)


## 自动生成ORM映射关系和序列化代码

第一步：设计对象并填写用于生成ORM的配置

```xml
<?xml version="1.0" encoding="UTF-8"?>
<tinyobj>
    <!--<import>tinyplayer.xml</import>-->

    <Weapon orm="0" keys="id" index="" comment="武器">
        <type    num="1" type="uint32"  comment="标识符"/>
        <name    num="2" type="string"  comment="名字"/>
    </Weapon>

    <Player keys="id" index="name" comment="角色信息">
        <id      num="1" type="uint32"  comment="标识符"/>
        <name    num="2" type="string"  default="david" comment="名字"/>
        <age num="3" type="uint8" comment="国家信息"/>
        <weapons num="4" type="std::vector{Weapon}" comment="武器列表"/>
    </Player>
</tinyobj>
```

第二步：生成ORM和序列化代码

```bash
tools/tinyobj.py -o example/tinyobj/ example/tinyobj/tinyplayer.xml
```

将会在`example/tinyobj`目录下生成下列文件：

- 对象定义文件：`tinyplayer.h` `tinyplayer.cpp`。
- 对象序列化相关文件：`tinyplayer.proto` `tinyplayer.pb.h` `tinyplayer.pb.cc`。
- ORM映射：`tinyplayer.orm.h`

例子详情，请查看：[tinyplayer.xml](./example/tinyobj/tinyplayer.xml)

## 自动生成配置文件说明

#### 字段类型支持（`type`字段）

- 基本类型：

```
    脚本类型 : (C++类型、PROTO类型、DB字段类型、默认值)

    # 整数-有符号
    "int8" : ("int8_t",  "sint32", "INT8", "0"),
    "int16": ("int16_t", "sint32", "INT16", "0"),
    "int32": ("int32_t", "sint32", "INT32", "0"),
    "int64": ("int64_t", "sint64", "INT64", "0"),

    # 整数-无符号
    "uint8" : ("uint8_t",  "uint32", "UINT8", "0"),
    "uint16": ("uint16_t", "uint32", "UINT16", "0"),
    "uint32": ("uint32_t", "uint32", "UINT32", "0"),
    "uint64": ("uint64_t", "uint64", "UINT64", "0"),

    # 浮点
    "float" : ("float", "float", "FLOAT", "0"),
    "double": ("double", "double", "DOUBLE", "0"),

    # 布尔
    "bool": ("bool", "bool", "BOOL", "0"),

    # 字符串
    "string": ("std::string", "bytes", "STRING", ""),
    "vchar" : ("std::string", "bytes", "VCHAR", ""),

    # 二进制
    "bytes"         : ("std::string", "bytes", "BYTES", ""),
    "bytes_tiny"    : ("std::string", "bytes", "BYTES_TINY", ""),
    "bytes_medium"  : ("std::string", "bytes", "BYTES_MEDIUM", ""),
    "bytes_long"    : ("std::string", "bytes", "BYTES_LONG",""),
```

- 自定义类型：

```
Player
Weapon
```

- STL：（由于`<`和`>`两个字符在XML中是特殊字符，脚本中表示STL容器时用`{`和`}`替换即可）

```
// 简单
std::vector{int8}
// 复合类型
std::list<Player>
// 任意层次的嵌套
std::map{std::map{std::string, Player}}
```

#### 指定表的键值和索引（现阶段尚未支持索引）  

```
<Player keys="id" index="name" comment="角色信息">
    ...
</Player>
```

#### 控制不需要生成ORM映射
 
`orm="0"`

```
<Weapon orm="0" keys="id" index="" comment="武器">
    ...
</Weapon>
```
 
## 参考
 
 - https://en.wikipedia.org/wiki/Object-relational_mapping
 - http://www.agiledata.org/essays/mappingObjects.html
 - http://www.codesynthesis.com/products/odb/
 
 