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

#ifndef TINYWORLD_TINYREFLECTION_H
#define TINYWORLD_TINYREFLECTION_H

#include "tinyworld.h"

#include <iostream>
#include <functional>
#include <vector>
#include <memory>
#include <unordered_map>
#include <typeinfo>
#include <type_traits>
#include <boost/any.hpp>

TINY_NAMESPACE_BEGIN


//
// Dummy Serializer, Do nothing, satisfy concept : Serializer
//
template<typename T>
struct DummySerializer {
    std::string serialize(const T &object) const { return ""; }

    bool deserialize(T &object, const std::string &bin) const { return false; }
};

//
// Base Property Reflection Class
//
template<typename T>
class Property {
public:
    typedef std::shared_ptr<Property> Ptr;

    Property(const std::string &name, uint16_t number)
            : name_(name), number_(number) {}

    const std::string &name() { return name_; }

    uint16_t number() { return number_; }

public:
    //
    // Getter & Setter
    //
    virtual const std::type_info &type() = 0;

    virtual boost::any get(const T &) = 0;

    virtual void set(T &, boost::any &) = 0;

    //
    // Serialization
    //
    virtual std::string serialize(const T &) = 0;

    virtual bool deserialize(T &object, const std::string &bin) = 0;

protected:
    std::string name_;
    uint16_t number_;
};


//
// Concrete Property Reflection
//
template<typename T, typename MemFn, typename SerializerT>
class Property_T : public Property<T> {
public:
    Property_T(const std::string &name, uint16_t id, MemFn fn)
            : Property<T>(name, id), fn_(fn) {}

    using PropRefType = typename std::result_of<MemFn(T &)>::type;
    using PropType = typename std::remove_reference<PropRefType>::type;

    virtual const std::type_info &type() final {
        return typeid(PropType);
    };

    boost::any get(const T &obj) final {
        return fn_(obj);
    }

    void set(T &obj, boost::any &v) final {
        fn_(obj) = boost::any_cast<PropType>(v);
    }

    std::string serialize(const T &obj) final {
        SerializerT serializer;
        return serializer.serialize(fn_(obj));
    }

    bool deserialize(T &obj, const std::string &data) final {
//        std::cout << __PRETTY_FUNCTION__ << std::endl;
        SerializerT serializer;
        return serializer.deserialize(fn_(obj), data);
    }

protected:
    MemFn fn_;
};

template<typename T, typename SerializerT, typename MemFn>
Property_T<T, MemFn, SerializerT> *makePropery(const std::string &name, uint16_t id, MemFn fn) {
    return new Property_T<T, MemFn, SerializerT>(name, id, fn);
}


//
// Structure Reflection
//
struct StructBase {
};

template<typename T>
struct Struct : public StructBase {
public:
    typedef typename Property<T>::Ptr PropertyPtr;
    typedef std::vector<PropertyPtr> PropertyContainer;
    typedef std::unordered_map<std::string, PropertyPtr> PropertyMap;
    typedef std::unordered_map<uint16_t, PropertyPtr> PropertyMapByID;

    Struct(const std::string &name, uint16_t version = 0)
            : name_(name), version_(version) {}

    virtual ~Struct() {}

    T *clone() { return new T; }

    template<template<typename> class SerializerT = DummySerializer, typename PropType>
    Struct<T> &property(const std::string &name, PropType T::* prop, uint16_t id = 0) {
        if (!hasPropery(name)) {
            typename Property<T>::Ptr ptr(makePropery<T, SerializerT<PropType>>(name, id, std::mem_fn(prop)));
            properties_[name] = ptr;
            properties_ordered_.push_back(ptr);

            if (id) {
                properties_byid_[id] = ptr;
            }
        }

        return *this;
    }

    Struct<T> &version(uint16_t ver) {
        version_ = ver;
        return *this;
    }

    bool hasPropery(const std::string &name) {
        return properties_.find(name) != properties_.end();
    }

    size_t propertyCount() { return properties_.size(); }

    PropertyContainer propertyIterator() { return properties_ordered_; }

    typename Property<T>::Ptr propertyByName(const std::string &name) {
        auto it = properties_.find(name);
        if (it != properties_.end())
            return it->second;
        return typename Property<T>::Ptr();
    }

    typename Property<T>::Ptr propertyByID(uint16_t id) {
        auto it = properties_byid_.find(id);
        if (it != properties_byid_.end())
            return it->second;
        return typename Property<T>::Ptr();
    }

    template<typename PropType>
    PropType get(const T &obj, const std::string &propname) {
        auto prop = propertyByName(propname);
        if (prop)
            return boost::any_cast<PropType>(prop->get(obj));
        return PropType();
    }

    template<typename PropType>
    void set(T &obj, const std::string &propname, const PropType &value) {
        auto prop = propertyByName(propname);
        if (prop) {
            boost::any v = value;
            prop->set(obj, v);
        }
    }

    const std::string &name() { return name_; }

    uint16_t version() { return version_; }

protected:
    std::string name_;
    PropertyContainer properties_ordered_;
    PropertyMap properties_;
    PropertyMapByID properties_byid_;

    uint16_t version_ = 0;
};


struct StructFactory {
    static StructFactory &instance() {
        static StructFactory instance_;
        return instance_;
    }

    template<typename T>
    Struct<T> &declare(const std::string name = "") {
        std::string type_name = typeid(T).name();
        std::string struct_name = name;
        if (name.empty())
            struct_name = type_name;

        auto desc = std::make_shared<Struct<T>>(struct_name);
        structs_by_typeid_[type_name] = desc;
        structs_by_name_[struct_name] = desc;
        return *desc;
    }

    template<typename T>
    Struct<T> *structByType() {
        std::string type_name = typeid(T).name();
        auto it = structs_by_typeid_.find(type_name);
        if (it != structs_by_typeid_.end())
            return static_cast<Struct<T> *>(it->second.get());
        return NULL;
    }

    template<typename T>
    Struct<T> *structByName(const std::string &name) {
        auto it = structs_by_name_.find(name);
        if (it != structs_by_name_.end())
            return static_cast<Struct<T> *>(it->second.get());
        return NULL;
    }

protected:
    typedef std::unordered_map<std::string, std::shared_ptr<StructBase>> Structs;

    Structs structs_by_typeid_;
    Structs structs_by_name_;
};

TINY_NAMESPACE_END

#endif //TINYWORLD_TINYREFLECTION_H
