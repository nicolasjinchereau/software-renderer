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

template<class T> inline JSONType JSONGetType();
template<> inline JSONType JSONGetType<JSONNull>()    { return JSONType::Null; }
template<> inline JSONType JSONGetType<JSONObject>()  { return JSONType::Object; }
template<> inline JSONType JSONGetType<JSONArray>()   { return JSONType::Array; }
template<> inline JSONType JSONGetType<JSONString>()  { return JSONType::String; }
template<> inline JSONType JSONGetType<JSONInteger>() { return JSONType::Integer; }
template<> inline JSONType JSONGetType<JSONFloat>()   { return JSONType::Float; }
template<> inline JSONType JSONGetType<JSONBoolean>() { return JSONType::Boolean; }

class JSONValue
{
    struct PayloadBase
    {
        PayloadBase(){}
        virtual ~PayloadBase(){}
        virtual PayloadBase* MoveConstruct(void* addr) const = 0;
        virtual PayloadBase* CopyConstruct(void* addr) const = 0;
        virtual JSONType GetType() const = 0;
        virtual void* GetAddress(JSONType expectedType) = 0;
        virtual std::string ToString() const = 0;

        template<class T> T& GetValue() {
            return *(T*)GetAddress(JSONGetType<T>());
        }

        template<class T> const T& GetValue() const {
            return *(const T*)((PayloadBase*)this)->GetAddress(JSONGetType<T>());
        }
    };

    template<class T>
    struct Payload : public PayloadBase
    {
        T obj;
        
        template<typename... Ts>
        Payload(Ts&&... ts) : obj(std::forward<Ts>(ts)...){}

        virtual PayloadBase* MoveConstruct(void* addr) const {
            return new (addr) Payload<T>(std::move(obj));
        }
        
        virtual PayloadBase* CopyConstruct(void* addr) const {
            return new (addr) Payload<T>(obj);
        }

        virtual JSONType GetType() const {
            return JSONGetType<T>();
        }

        virtual void* GetAddress(JSONType expectedType)
        {
            if(JSONGetType<T>() != expectedType)
                throw JSONException("The contained object is not of type '"s + TypeToString(expectedType) + "'");

            return (void*)&obj;
        }

        template<class T> std::string _toString() const;
        template<> std::string _toString<JSONNull>() const { return "null"; }
        template<> std::string _toString<JSONObject>() const { return "Object"; }
        template<> std::string _toString<JSONArray>() const { return "Array"; }
        template<> std::string _toString<JSONString>() const { return obj; }
        template<> std::string _toString<JSONInteger>() const { return to_string(obj); }
        template<> std::string _toString<JSONFloat>() const {
            char buff[3 + DBL_MANT_DIG - DBL_MIN_EXP];
            sprintf_s(buff, "%g", obj);
            return buff;
        }
        template<> std::string _toString<JSONBoolean>() const { return obj ? "true" : "false"; }

        virtual std::string ToString() const {
            return _toString<T>();
        }
    };

    union PayloadData {
        Payload<JSONObject>  _object;
        Payload<JSONArray>   _array;
        Payload<JSONString>  _string;
        Payload<JSONInteger> _integer;
        Payload<JSONFloat>   _float;
        Payload<JSONBoolean> _boolean;
        PayloadData(){}
        ~PayloadData(){}
    };

    PayloadData _payload;
    PayloadBase& payload() { return *(PayloadBase*)&_payload; }
    const PayloadBase& payload() const { return *(PayloadBase*)&_payload; }

    template<class T, class U>
    void _construct(U&& x) {
        new (&_payload) Payload<T>(std::forward<U>(x));
    }

    void _destroy() {
        ((PayloadBase*)&_payload)->~PayloadBase();
    }

    void _copyFrom(const JSONValue& x) {
        x.payload().CopyConstruct(&_payload);
    }

    void _moveFrom(JSONValue&& x) {
        x.payload().MoveConstruct(&_payload);
    }

    static std::string TypeToString(JSONType type);
    
public:

    JSONValue() {
        _construct<JSONNull>(nullptr);
    }

    JSONValue::JSONValue(JSONValue&& x) {
        _moveFrom(std::move(x));
    }

    JSONValue(const JSONValue& x) {
        _copyFrom(x);
    }

    JSONValue(nullptr_t) {
        _construct<JSONNull>(nullptr);
    }

    JSONValue(const JSONObject& x) {
        _construct<JSONObject>(x);
    }

    JSONValue(JSONObject&& x) {
        _construct<JSONObject>(std::move(x));
    }

    JSONValue(const JSONArray& x) {
        _construct<JSONArray>(x);
    }

    JSONValue(JSONArray&& x) {
        _construct<JSONArray>(std::move(x));
    }

    JSONValue(const std::string& x) {
        _construct<JSONString>(x);
    }

    JSONValue(std::string&& x) {
        _construct<JSONString>(std::move(x));
    }

    JSONValue(const char* x) {
        _construct<JSONString>(x);
    }

    JSONValue(int x) {
        _construct<JSONInteger>(x);
    }

    JSONValue(long long x) {
        _construct<JSONInteger>(x);
    }

    JSONValue(double x) {
        _construct<JSONFloat>(x);
    }

    JSONValue(bool x) {
        _construct<JSONBoolean>(x);
    }

    ~JSONValue() {
        _destroy();
    }

    JSONValue& operator=(const JSONValue& x) {
        _destroy();
        _copyFrom(x);
        return *this;
    };

    JSONValue& operator=(JSONValue&& x) {
        _destroy();
        _moveFrom(std::forward<JSONValue>(x));
        return *this;
    };
    
    operator JSONObject&() {
        return payload().GetValue<JSONObject>();
    }

    operator JSONArray&() {
        return payload().GetValue<JSONArray>();
    }

    operator JSONString() {
        return payload().GetValue<JSONString>();
    }

    operator int() {
        return (int)payload().GetValue<JSONInteger>();
    }

    operator long long() {
        return payload().GetValue<JSONInteger>();
    }
    
    operator float() {
        return (float)payload().GetValue<JSONFloat>();
    }

    operator double() {
        return payload().GetValue<JSONFloat>();
    }

    operator bool() {
        return payload().GetValue<JSONBoolean>();
    }

    JSONObject& asObject() {
        return payload().GetValue<JSONObject>();
    }

    JSONArray& asArray() {
        return payload().GetValue<JSONArray>();
    }

    JSONString& asString() {
        return payload().GetValue<JSONString>();
    }

    long long& asInteger() {
        return payload().GetValue<JSONInteger>();
    }

    double& asFloat() {
        return payload().GetValue<JSONFloat>();
    }

    bool& asBoolean() {
        return payload().GetValue<JSONBoolean>();
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
        auto it = payload().GetValue<JSONObject>().emplace(key, JSONValue());
        return it.first->second;
    }

    const JSONValue& operator[](const char* key) const {
        return (*(JSONValue*)this)[key];
    }

    JSONValue& operator[](int index) {
        return payload().GetValue<JSONArray>()[index];
    }

    const JSONValue& operator[](int index) const {
        return payload().GetValue<JSONArray>()[index];
    }

    void Append(const JSONValue& x) {
        return payload().GetValue<JSONArray>().push_back(x);
    }

    void Append(JSONValue&& x) {
        return payload().GetValue<JSONArray>().push_back(std::move(x));
    }

    JSONType valueType() const {
        return payload().GetType();
    }

    std::string ToString() const {
        return payload().ToString();
    }

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
