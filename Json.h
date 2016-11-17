/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <string>
#include <cstdlib>
#include <unordered_map>
#include <vector>
#include <utility>
using namespace std::string_literals;

enum class JSONType
{
    Null,
    Object,
    Array,
    String,
    Integer,
    Float,
    Boolean
};

class JSONValue;

typedef std::nullptr_t                             JSONNull;
typedef std::unordered_map<std::string, JSONValue> JSONObject;
typedef std::vector<JSONValue>                     JSONArray;
typedef std::string                                JSONString;
typedef long long                                  JSONInteger;
typedef double                                     JSONFloat;
typedef bool                                       JSONBoolean;

class JSONValue
{
    union Payload {
        JSONObject  _object;
        JSONArray   _array;
        JSONString  _string;
        JSONInteger _integer;
        JSONFloat   _float;
        JSONBoolean _boolean;

        Payload(){}
        ~Payload(){}
    };

    JSONType type = JSONType::Null;
    Payload payload;

    void CopyFrom(const JSONValue& x);
    void ConstructFrom(JSONValue&& x);
    void Clear();

    void ThrowIfNotType(JSONType type) const;
    static std::string TypeToString(JSONType type);
public:
    JSONValue(){}
    JSONValue(JSONValue&& x);
    JSONValue(const JSONValue& x);
    JSONValue(nullptr_t);
    JSONValue(const JSONObject& x);
    JSONValue(JSONObject&& x);
    JSONValue(const JSONArray& x);
    JSONValue(JSONArray&& x);
    JSONValue(const std::string& x);
    JSONValue(std::string&& x);
    JSONValue(const char* x);
    JSONValue(int x);
    JSONValue(long long x);
    JSONValue(double x);
    JSONValue(bool x);
    ~JSONValue();

    JSONValue& operator=(const JSONValue& x) {
        CopyFrom(x);
        return *this;
    };

    JSONValue& operator=(JSONValue&& x) {
        Clear();
        ConstructFrom(std::forward<JSONValue>(x));
        return *this;
    };
    
    operator JSONObject&() {
        ThrowIfNotType(JSONType::Object);
        return payload._object;
    }

    operator JSONArray&() {
        ThrowIfNotType(JSONType::Array);
        return payload._array;
    }

    operator std::string() {
        ThrowIfNotType(JSONType::String);
        return payload._string;
    }

    operator int() {
        ThrowIfNotType(JSONType::Integer);
        return (int)payload._integer;
    }

    operator long long() {
        ThrowIfNotType(JSONType::Integer);
        return payload._integer;
    }

    operator float() {
        ThrowIfNotType(JSONType::Float);
        return (float)payload._float;
    }

    operator double() {
        ThrowIfNotType(JSONType::Float);
        return payload._float;
    }

    operator bool() {
        ThrowIfNotType(JSONType::Boolean);
        return payload._boolean;
    }

    JSONObject& asObject() {
        ThrowIfNotType(JSONType::Object);
        return payload._object;
    }

    JSONArray& asArray() {
        ThrowIfNotType(JSONType::Array);
        return payload._array;
    }

    std::string& asString() {
        ThrowIfNotType(JSONType::String);
        return payload._string;
    }

    long long& asInteger() {
        ThrowIfNotType(JSONType::Integer);
        return payload._integer;
    }

    double& asFloat() {
        ThrowIfNotType(JSONType::Float);
        return payload._float;
    }

    bool& asBoolean() {
        ThrowIfNotType(JSONType::Boolean);
        return payload._boolean;
    }

    const JSONObject& asObject() const {
        return ((JSONValue*)this)->asObject();
    }

    const JSONArray& asArray() const {
        return ((JSONValue*)this)->asArray();
    }

    const std::string& asString() const {
        return ((JSONValue*)this)->asString();
    }

    const long long& asInteger() const {
        return ((JSONValue*)this)->asInteger();
    }

    const double& asFloat() const {
        return ((JSONValue*)this)->asFloat();
    }

    const bool& asBoolean() const {
        return ((JSONValue*)this)->asBoolean();
    }

    JSONValue& operator[](const char* key) {
        ThrowIfNotType(JSONType::Object);
        auto it = payload._object.emplace(key, JSONValue());
        return it.first->second;
    }

    const JSONValue& operator[](const char* key) const {
        return (*(JSONValue*)this)[key];
    }

    JSONValue& operator[](int index) {
        ThrowIfNotType(JSONType::Array);
        return payload._array[index];
    }

    const JSONValue& operator[](int index) const {
        return (*(JSONValue*)this)[index];
    }

    void Append(const JSONValue& x) {
        ThrowIfNotType(JSONType::Array);
        payload._array.push_back(x);
    }

    void Append(JSONValue&& x) {
        ThrowIfNotType(JSONType::Array);
        payload._array.push_back(std::forward<JSONValue>(x));
    }

    JSONType valueType() const {
        return type;
    }

    std::string ToString() const;

    friend std::ostream& operator<<(std::ostream& os, const JSONValue& obj) {
        os << obj.ToString();
        return os;
    }
};

class JSONParser
{
public:
    static JSONValue Parse(const std::string& json);
    static JSONValue Load(const std::string& filename);
    static std::string Dump(const JSONValue& value, bool pretty);

private:
    struct Iterator
    {
        size_t offset;
        size_t line;
        size_t column;
        const char* value;

        Iterator(const char* json);

        // advance to next non-whitespace character
        // after skipping current character
        Iterator& operator++();
        Iterator operator++(int);

        // advance or rewind by N characters
        // ignoring character type
        Iterator operator+(size_t count) const;
        Iterator& operator+=(size_t count);
        Iterator operator-(size_t count) const;
        Iterator& operator-=(size_t count);

        const char& operator*() const;
        const char& operator[](size_t index) const;
        std::string location() const;
        std::string enclosingLine() const;
    private:
        void Skip();
        void Advance();
    };

    static JSONValue ParseValue(Iterator& iter);
    static JSONValue ParseObject(Iterator& iter);
    static JSONValue ParseArray(Iterator& iter);
    static JSONValue ParseString(Iterator& iter);
    static JSONValue ParseNumber(Iterator& iter);
    static JSONValue ParseBoolean(Iterator& iter);
    static JSONValue ParseNull(Iterator& iter);
    static void Dump(const JSONValue& value, std::stringstream& output, bool pretty, int depth = 0);
    static bool IsNumChar(char c);
    static bool StrCmpI(const char* a, const char* b, size_t n);

    friend class JSONException;
};

class JSONException : public std::exception
{
    std::string msg;
public:
    JSONException(const std::string& message);
    JSONException(const JSONParser::Iterator& iter, const std::string& message);
    virtual char const* what() const override;
};
