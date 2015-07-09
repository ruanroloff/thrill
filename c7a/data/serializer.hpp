/*******************************************************************************
 * c7a/data/serializer.hpp
 *
 * Part of Project c7a.
 *
 *
 * This file has no license. Only Chuck Norris can compile it.
 ******************************************************************************/

#pragma once
#ifndef C7A_DATA_SERIALIZER_HEADER
#define C7A_DATA_SERIALIZER_HEADER

#include <string>
#include <cstring>
#include <utility>
#include <cassert>
#include <tuple>

#include <c7a/common/logger.hpp>

//TODO DELETE
#include <iostream>
#include <tests/data/serializer_objects.hpp>
#include <build/c7a/proto/test_serialize_object.pb.h>

//TODO CEREAL
#include <cereal/archives/binary.hpp>
#include <sstream>

//TODO(ts) this copies data. That is bad and makes me sad.

namespace c7a {
namespace data {

//! internal representation of data elements
typedef std::string Blob;

//! \namespace namespace to hide the implementations of serializers
namespace serializers {

template <class T>
struct Impl;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////          BASIC SERIALIZERS          //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! identity serializer from string to string
template <>
struct Impl<std::string>{
    static std::string Serialize(const std::string& x) {
        return x;
    }

    static std::string Deserialize(const std::string& x) {
        return x;
    }
};

//! serializer for int
template <>
struct Impl<int>{
    static std::string Serialize(const int& x) {
        return std::to_string(x);
    }

    static int Deserialize(const std::string& x) {
        return std::stoi(x);
    }
};

//! serializer for long
template <>
struct Impl<long>{
    static std::string Serialize(const long& x) {
        return std::to_string(x);
    }

    static long Deserialize(const std::string& x) {
        return std::stol(x);
    }
};

//! serializer for long long
template <>
struct Impl<long long>{
    static std::string Serialize(const long long& x) {
        return std::to_string(x);
    }

    static long long Deserialize(const std::string& x) {
        return std::stoll(x);
    }
};

//! serializer for unsigned
template <>
struct Impl<unsigned int>{
    static std::string Serialize(const unsigned int& x) {
        return std::to_string(x);
    }

    static unsigned int Deserialize(const std::string& x) {
        return std::stoul(x);
        //TODO: WHY IS THIS WORKING???? I DONT UNDERSTAND
    }
};

//! serializer for unsigned long
template <>
struct Impl<unsigned long>{
    static std::string Serialize(const unsigned long& x) {
        return std::to_string(x);
    }

    static unsigned long Deserialize(const std::string& x) {
        return std::stoul(x);
    }
};

//! serializer for unsigned long long
template <>
struct Impl<unsigned long long>{
    static std::string Serialize(const unsigned long long& x) {
        return std::to_string(x);
    }

    static unsigned long long Deserialize(const std::string& x) {
        return std::stoull(x);
    }
};

//! serializer for float
template <>
struct Impl<float>{
    static std::string Serialize(const float& x) {
        return std::to_string(x);
    }

    static float Deserialize(const std::string& x) {
        return std::stof(x);
    }
};

//! serializer for double
template <>
struct Impl<double>{
    static std::string Serialize(const double& x) {
        return std::to_string(x);
    }

    static double Deserialize(const std::string& x) {
        return std::stod(x);
    }
};

//! serializer for long double
template <>
struct Impl<long double>{
    static std::string Serialize(const long double& x) {
        return std::to_string(x);
    }

    static long double Deserialize(const std::string& x) {
        return std::stold(x);
    }
};

//! serializer for (string, int) tuples
template <>
struct Impl<std::pair<std::string, int> >{
    static std::string Serialize(const std::pair<std::string, int>& x) {
        std::size_t len = sizeof(int) + x.first.size();
        char result[len];
        std::memcpy(result, &(x.second), sizeof(int));
        std::memcpy(result + sizeof(int), x.first.c_str(), x.first.size());
        return std::string(result, len);
    }
    //TODO(tb): What exactly happens with c_string. What is thiiiiiis?
    static std::pair<std::string, int> Deserialize(const std::string& x) {
        int i;
        std::size_t str_len = x.size() - sizeof(int);
        std::memcpy(&i, x.c_str(), sizeof(int));
        std::string s(x, sizeof(int), str_len);
        return std::make_pair(s, i);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////          PAIR SERIALIZER          ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO(cn): do we have clusternodes working on 32 and 64bit systems at the same time??
//! serializer for pairs
template <typename T1, typename T2>
struct Impl<std::pair<T1, T2> >{
    static std::string Serialize(const std::pair<T1, T2>& x) {
        // UINT_MAX not working on Jenkins o.o
        // if( x.first.size() > UINT_MAX ) {
        //     //TODO ERROR
        // }
        std::string t1 = serializers::Impl<T1>::Serialize(x.first);
        std::string t2 = serializers::Impl<T2>::Serialize(x.second);
        uint16_t len_t1 = t1.size();

        std::size_t len = t1.size() + t2.size() + sizeof(uint16_t);
        char result[len];
        std::memcpy(result, &len_t1, sizeof(uint16_t));
        std::memcpy(result + sizeof(uint16_t), t1.c_str(), t1.size());
        std::memcpy(result + sizeof(uint16_t) + t1.size(), t2.c_str(), t2.size());
        //resulting string is: len(x.first) + serialized(x.first) + serialized(x.second)
        return std::string(result, len);
    }
    static std::pair<T1, T2> Deserialize(const std::string& x) {
        uint16_t len_t1;
        std::memcpy(&len_t1, x.c_str(), sizeof(uint16_t));
        std::size_t len_t2 = x.size() - sizeof(uint16_t) - len_t1;

        std::string t1_str = x.substr(sizeof(uint16_t), len_t1);
        std::string t2_str = x.substr(sizeof(uint16_t) + len_t1, len_t2);

        T1 t1 = serializers::Impl<T1>::Deserialize(t1_str);
        T2 t2 = serializers::Impl<T2>::Deserialize(t2_str);

        return std::make_pair(t1, t2);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////          TUPLE SERIALIZER          ///////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct TupleHelper {
    template <std::size_t>
    struct len_tuple { };

    template <typename Tuple, size_t Id>
    static std::string SerializeRecursively(const Tuple& t, len_tuple<Id>) {
        auto elem = std::get<std::tuple_size<Tuple>::value - Id>(t);
        auto s = serializers::Impl<decltype(elem)>::Serialize(elem);
        uint16_t len_s = s.size();
        auto tail_s = SerializeRecursively(t, len_tuple<Id - 1>());

        std::size_t len = s.size() + tail_s.size() + sizeof(uint16_t);
        char result[len];
        std::memcpy(result, &len_s, sizeof(uint16_t));
        std::memcpy(result + sizeof(uint16_t), s.c_str(), s.size());
        std::memcpy(result + sizeof(uint16_t) + s.size(), tail_s.c_str(), tail_s.size());

        return std::string(result, len);
    }

    template <typename Tuple>
    static std::string SerializeRecursively(const Tuple& t, len_tuple<1>/*i am useless*/) {
        auto elem = std::get<std::tuple_size<Tuple>::value - 1>(t);
        auto s = serializers::Impl<decltype(elem)>::Serialize(elem);
        uint16_t len_s = s.size();

        std::size_t len = s.size() + sizeof(uint16_t);
        char result[len];
        std::memcpy(result, &len_s, sizeof(uint16_t));
        std::memcpy(result + sizeof(uint16_t), s.c_str(), s.size());

        return std::string(result, len);
    }

    template <typename T, typename ... Tail>
    static std::tuple<T, Tail ...> DeserializeRecursively(const std::string& x, len_tuple<1>) {
        uint16_t len_elem;
        std::memcpy(&len_elem, x.c_str(), sizeof(uint16_t));
        const std::string elem_str = x.substr(sizeof(uint16_t), len_elem);
        const T elem = serializers::Impl<T>::Deserialize(elem_str);

        return std::make_tuple(elem);
    }

    template <typename T, typename ... Tail, size_t Id>
    static std::tuple<T, Tail ...> DeserializeRecursively(const std::string& x, len_tuple<Id>) {
        uint16_t len_elem;
        std::memcpy(&len_elem, x.c_str(), sizeof(uint16_t));
        const std::string elem_str = x.substr(sizeof(uint16_t), len_elem);
        const T elem = serializers::Impl<T>::Deserialize(elem_str);

        const std::string x_tail = x.substr(len_elem + sizeof(uint16_t));
        auto tuple_tail = DeserializeRecursively<Tail ...>(x_tail, len_tuple<Id - 1>());

        return std::tuple_cat(std::make_tuple(elem), tuple_tail);
    }
};

template <typename ... Args>
struct Impl<std::tuple<Args ...> >{

    static std::string Serialize(const std::tuple<Args ...>& t) {
        return TupleHelper::SerializeRecursively(t, TupleHelper::len_tuple<sizeof ... (Args)>());
    }

    static std::tuple<Args ...> Deserialize(const std::string& x) {
        return TupleHelper::DeserializeRecursively<Args ...>(x, TupleHelper::len_tuple<sizeof ... (Args)>());
    }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////          PROTOBUF          ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <>
struct Impl<struct TestSerializeObject>{
    static std::string Serialize(const struct TestSerializeObject& t) {
        serial::TestSerializeObject tso;
        tso.set_bla_(t.bla_);
        tso.set_blu_(t.blu_);
        std::string res;
        tso.SerializeToString(&res);
        return res;
        return "";
    }
    static TestSerializeObject Deserialize(const std::string& x) {
        serial::TestSerializeObject tso;
        tso.ParseFromString(x);
        struct TestSerializeObject pt(tso.bla_(), tso.blu_());
        return pt;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////          CEREAL          ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <>
struct Impl<struct TestCerealObject>{
    static std::string Serialize(const struct TestCerealObject& t) {
        std::stringstream ss; // any stream can be used

        cereal::BinaryOutputArchive oarchive(ss); // Create an output archive
        oarchive(t); // Write the data to the archive

        return ss.str();
    }

    static TestCerealObject Deserialize(const std::string& t) {
        std::stringstream ss;
        ss.str (t);
        cereal::BinaryInputArchive iarchive(ss); // Create an input archive

        TestCerealObject res;
        iarchive(res); // Read the data from the archive
        return res;
    }

};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! binary serializer for any integral type, usable as template.
template <typename Type>
struct GenericImpl {
    static std::string Serialize(const Type& v) {
        std::stringstream ss; // any stream can be used

        cereal::BinaryOutputArchive oarchive(ss); // Create an output archive
        oarchive(v); // Write the data to the archive

        return ss.str();
    }

    static Type        Deserialize(const std::string& s) {
        std::stringstream ss;
        ss.str (s);
        cereal::BinaryInputArchive iarchive(ss); // Create an input archive

        Type res;
        iarchive(res); // Read the data from the archive
        return res;
    }
};
}       // namespace serializers

//! Serialize the type to std::string
template <class T>
inline std::string Serialize(const T& x) {
    return serializers::Impl<T>::Serialize(x);
}

//! Deserialize the std::string to the given type
template <class T>
inline T Deserialize(const std::string& x) {
    return serializers::Impl<T>::Deserialize(x);
}
} // namespace data
} // namespace c7a

#endif // !C7A_DATA_SERIALIZER_HEADER

/******************************************************************************/
