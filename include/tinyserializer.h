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

// Serializer and seriailze/deseriaize declaration

#ifndef TINYWORLD_TINYSERIALIZER_H
#define TINYWORLD_TINYSERIALIZER_H

#include "tinyworld.h"

TINY_NAMESPACE_BEGIN

//
// Concept : [Serializer]
//
// Constraint:
//    template<typename T>
//    struct Serializer {
//       std::string serialize(const T &object) const;
//       bool deserialize(T &object, const std::string &bin) const;
//    };
//
// Such as: ProtoSerialzer, ProtoDynSerializer
//
// Usage:
//   Serializer serializer;
//   Player p1;
//   std::string data = serializer.serialize(p);
//
//   Player p2;
//   serializer.deserializer(p2, data);
//

template <typename T>
class ProtoSerializer;

template <typename T>
class ProtoDynSerializer;

//
// serialize VS. deserialize functions
//
// eg.
//
//    Player p;
//    // Serialize and deserialize by default serializer(ProtoSerializer)
//    void usage_1() {
//        std::string data = serialize(p);
//        deserialize(p, data);
//    }
//
//    // Serialize and deserialize by ProtoSerializer serializer
//    void usage_2() {
//        std::string data = serialize<ProtoDynSerializer>(p);
//        deserialize<ProtoDynSerializer>(p, data);
//    }
//
//    // Serializer has non-default constructor
//    void usage_3() {
//        DynProtoSerializer serializer(&GeneratedProtoMappingFactory::intance());
//        std::string data = serialize(p, serializer);
//        deserialize(p, serializer);
//    }
//
//    // Create a new Serializer
//    void usage_4() {
//        std::string data = serialize<JsonSerializer>(p);
//        deserialize<JsonSerializer>(p, data);
//    }
//

template<template<typename T> class SerializerT = ProtoSerializer, typename T>
inline std::string serialize(const T &object, const SerializerT<T> &serializer = SerializerT<T>()) {
    return serializer.serialize(object);
};

template<template<typename T> class SerializerT = ProtoSerializer, typename T>
inline bool deserialize(T &object, const std::string &bin, const SerializerT<T> &serializer = SerializerT<T>()) {
    return serializer.deserialize(object, bin);
}

TINY_NAMESPACE_END


#endif //TINYWORLD_TINYSERIALIZER_H
