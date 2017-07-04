TinySerializer框架的用法
--------------------

## 0. 依赖

 - `C++11`
 - `protobuf`
 - `boost/any.hpp`

## 1. 用法（静态的`ProtoSerializer`）

注意：本节演示代码位于该项目的：`exmaple/demo_serialize.cpp`。

### 1.1 序列化基本类型

**支持下面的类型：**

- 整数：`uint8_t/int8_t`, `uint16_t/int16_t`, `int32_t/uint32_t`, `int64_t/uint64_t`, `bool`。
- 浮点数：`float`, `double`。
- 字符串：`std::string`。（注意：不支持C风格的字符串）

注意：同种类型的可以随意调整，不会影响序列化。如：uint32_t序列化的数据，使用uint8_t/uint64_t也可以进行正常的反序列化。

**例子：**

- 整数
    ```c++
            uint32_t v1 = 1024;
            std::string data = serialize(v1);
    
            uint64_t v2 = 0;
            deserialize(v2, data);
    
            std::cout << v2 << std::endl;
    ```
- 字符串

    ```c++
            std::string v1 = "Hello David++!";
            std::string data = serialize(v1);
    
            std::string v2;
            if (deserialize(v2, data))
                std::cout << v2 << std::endl;
            else
                std::cout << "error happens !" << std::endl;
    ```

### 1.2 序列化Protobuf生成的结构

```c++
        PlayerProto p1;
        p1.set_id(1024);
        p1.set_name("david");
        p1.add_quests(1);
        p1.add_quests(2);
        p1.mutable_weapon()->set_type(2);
        p1.mutable_weapon()->set_name("Sword");


        std::string data = serialize(p1);

        PlayerProto p2;
        if (deserialize(p2, data))
            std::cout << p2.ShortDebugString() << std::endl;
        else
            std::cout << "error happens !" << std::endl;
```

### 1.3 序列化STL容器

**支持的容器如下：**
 
 - Sequence: `vector`, `list`, `deque`。
 - Set: `set`, `multiset`。
 - Map: `map`, `multimap`。
 - HashSet: `unordered_set`, `unordered_multiset`。
 - HashMap: `unordered_map`, `unordered_multimap`。

 
 注意：
 - 同种类型的容器也是可以互换，而不影响序列化。
 - 支持容器的任意组合和嵌套。
 
 **例子：**
 
- 简单容器：`vector<uint8_t>`序列化的数据，使用`list<uint32_t>`进行反序列化

    ```c++
         std::vector<uint8_t> v1 = {1, 2, 3, 4, 5, 6};
         std::string data = serialize(v1);
    
         std::list<uint32_t> v2;
         deserialize(v2, data);
    
         for (auto &v : v2)
             std::cout << v << ",";
         std::cout << std::endl;
    ```
- 容器嵌套：`map`嵌套`vector`示例

    ```c++
        std::map<uint32_t, std::vector<PlayerProto>> v1 = {
                {1024, {p, p, p}},
                {1025, {p, p}},
                {1026, {p}},
        };
    
        std::string data = serialize(v1);
    
        std::map<uint32_t, std::vector<PlayerProto>> v2;
        deserialize(v2, data);
    
        for (auto &v : v2) {
            std::cout << v.first << std::endl;
            for (auto& player : v.second)
                std::cout << "\t - " << player.ShortDebugString() << std::endl;
        }
    ```


### 1.4 序列化用户自定义类型

对用户自定义类型序列化的支持，分两种方式：

 - 侵入式：实现`serialize`和`deserialize`成员函数。
 - 非侵入式：对该用户类型的`Serializer`进行偏特化。

**例子**

- 侵入式演示：
    
    ```c++
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
    
    // serialize & deserialize
    
    Weapon w;
    w.type = 22;
    w.name = "Blade";

    std::string data = serialize(w);

    Weapon w2;
    deserialize(w2, data);
    std::cout << w2.type << " - " << w2.name << std::endl;
    
    ```

- 非侵入式演示：

    ```c++
    struct Player {
        uint32_t id = 0;
        std::string name = "";
        std::map<uint32_t, std::vector<Weapon>> weapons_map;
    };
    
    // non-intrusive way
    template<>
    struct ProtoSerializer<Player> {
        std::string serialize(const Player &p) const {
            PlayerProto proto;
            proto.set_id(p.id);
            proto.set_name(p.name);
            // complex object
            proto.set_weapons_map(::serialize(p.weapons_map));
            return proto.SerializeAsString();
        }
    
        bool deserialize(Player &p, const std::string &data) const {
            PlayerProto proto;
            if (proto.ParseFromString(data)) {
                p.id = proto.id();
                p.name = proto.name();
                // complex object
                ::deserialize(p.weapons_map, proto.weapons_map());
                return true;
            }
            return false;
        }
    };
    
    // serialize & deserialize
    Player p;
    p.init();

    std::string data = serialize(p);

    Player p2;
    deserialize(p2, data);
    p2.dump();
    ```

## 2. 反射式用法（动态的`ProtoDynSerializer`）

对于基本类型、STL、Protobuf生成的类型，`ProtoDynSerializer`和`ProtoSerializer`的处理是一样的，区别在于用户定义类型的用法。

注意：
- 序列化和反序列化调用形式基本是一致的，只是`serialize`和`deserialize`函数多了一个模板参数`ProtoDynSerializer`。
- 本节演示代码位于：`exmaple/demo_serializer_dyn.cpp`。

### 2.1 基本类型、STL、Protobuf生成类型的序列化

- 以Probuf生成类型为例，其他的类似：

    ```c++
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
    ```


### 2.2 用户自定义类型的序列化

- 用户自定义类型：

    ```c++
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
    };
    ```
- 用户自定义类型到`Proto`的映射

    ```c++
    // Mapping
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
    ```
    
    执行`ProtoMappingFactory::instance().createAllProtoDefine()`，可以得到它们映射的Proto定义：
    
    ```proto
    // Weapon -> WeaponDynProto
    message WeaponDynProto {
      optional bytes type = 1;
      optional bytes name = 2;
    }
    
    // Player -> PlayerDynProto
    message PlayerDynProto {
      optional bytes id = 1;
      optional bytes name = 2;
      optional bytes quests = 3;
      optional bytes weapon = 4;
      optional bytes weapon2 = 5;
      optional bytes weapons_map = 6;
    }
    ```

- 序列化/反序列化

    ```c++
    try {
        Player p;
        p.init();
    
        std::string data = serialize<ProtoDynSerializer>(p);
    
        Player p2;
        deserialize<ProtoDynSerializer>(p2, data);
        p2.dump();
    } catch (const std::exception& e) {
        std::cout << "Error Happens: " << e.what() << std::endl;
    }
    ```

## 3.静态 vs. 动态

- 推荐使用静态玩法：
     - 静态执行效率高于动态。
     - 静态出错会在编译器暴露，而动态出错时会在运行时抛出异常。
     
- 动态的好处：用户自定义类型使用起来更加方便，不要额外定义Proto和实现序列化的约定。


- 性能简单对比：动态/静态 ~= 1.3倍

    ```bash
    $ time `./demo_serialize_dyn 10000 > /dev/null`
    
    real    0m7.522s
    user    0m7.051s
    sys     0m0.435s
    
    $ time `./demo_serialize 10000 > /dev/null`
    
    real    0m5.858s
    user    0m5.460s
    sys     0m0.373s
    ```



### 4. 进一步了解

 - [TinySerializer的设计](./tiny-serializer-design.md)
 
 