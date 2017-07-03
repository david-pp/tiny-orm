TinySerializer框架的设计
--------------------

## 1. 序列化器

- 形式如下即可：

    ```c++
    template<typename T>
    struct Serializer {
       std::string serialize(const T &object) const;
       bool deserialize(T &object, const std::string &bin) const;
    };
    ```

- 约定：
    - `serialize`函数负责把传入的对象转换成字节流，返回空则失败。
    - `deserialize`函数负责把字节流转换成传入的对象，返回`false`则失败。

- 默认实现了：
    - 静态玩法的`ProtoSerializer`。
    - 动态反射式玩法的`ProtoDynSerializer`。

## 2. 序列化函数

序列化器的帮助函数，对序列化器进行了简单封装。

### 2.1 定义：

```c++
template<template<typename T> class SerializerT = ProtoSerializer, typename T>
inline std::string serialize(const T &object, const SerializerT<T> &serializer = SerializerT<T>()) {
    return serializer.serialize(object);
};

template<template<typename T> class SerializerT = ProtoSerializer, typename T>
inline bool deserialize(T &object, const std::string &bin, const SerializerT<T> &serializer = SerializerT<T>()) {
    return serializer.deserialize(object, bin);
}

```

### 2.2 使用方式：

- 使用默认的序列化器进行序列化（`ProtoSerializer）
    ```c++
        Player p;
        // Serialize and deserialize by default serializer(ProtoSerializer)
        void usage_1() {
            std::string data = serialize(p);
            deserialize(p, data);
        }
    ```
- 指定序列化器进行序列化（`ProtoDynSerializer`）
    ```c++
    // Serialize and deserialize by ProtoSerializer serializer
    void usage_2() {
        std::string data = serialize<ProtoDynSerializer>(p);
        deserialize<ProtoDynSerializer>(p, data);
    }
    ```
    
- 序列化器构造函数可以传入参数

    ```c++
    // Serializer has non-default constructor
    void usage_3() {
        DynProtoSerializer serializer(&GeneratedProtoMappingFactory::intance());
        std::string data = serialize(p, serializer);
        deserialize(p, serializer);
    }
    ```

- 创建一个新的序列化器`JsonSerializer`

    ```c++
    // Create a new Serializer
    void usage_4() {
        std::string data = serialize<JsonSerializer>(p);
        deserialize<JsonSerializer>(p, data);
    }
    ```
   