/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#include "Json.h"
#include <sstream>
#include <fstream>
using namespace std;

////////////////////////////////
//    JSONValue
////////////////////////////////

JSONValue::JSONValue(JSONValue&& x) {
    ConstructFrom(std::forward<JSONValue>(x));
}

JSONValue::JSONValue(const JSONValue& x) {
    CopyFrom(x);
}

JSONValue::JSONValue(nullptr_t) {
    Clear();
}

JSONValue::JSONValue(const JSONObject& x) {
    new (&payload._object) JSONObject(x);
    type = JSONType::Object;
}

JSONValue::JSONValue(JSONObject&& x) {
    new (&payload._object) JSONObject(forward<JSONObject>(x));
    type = JSONType::Object;
}

JSONValue::JSONValue(const JSONArray& x) {
    new (&payload._array) JSONArray(x);
    type = JSONType::Array;
}

JSONValue::JSONValue(JSONArray&& x) {
    new (&payload._array) JSONArray(forward<JSONArray>(x));
    type = JSONType::Array;
}

JSONValue::JSONValue(const std::string& x) {
    new (&payload._string) string(x);
    type = JSONType::String;
}

JSONValue::JSONValue(std::string&& x)
{
    new (&payload._string) string(forward<string>(x));
    type = JSONType::String;
}

JSONValue::JSONValue(const char* x)
{
    new (&payload._string) string(x);
    type = JSONType::String;
}

JSONValue::JSONValue(int x) {
    payload._integer = (long long)x;
    type = JSONType::Integer;
}

JSONValue::JSONValue(long long x)
{
    payload._integer = x;
    type = JSONType::Integer;
}

JSONValue::JSONValue(double x) {
    payload._float = x;
    type = JSONType::Float;
}

JSONValue::JSONValue(bool x) {
    payload._boolean = x;
    type = JSONType::Boolean;
}

JSONValue::~JSONValue() {
    Clear();
}

string JSONValue::TypeToString(JSONType type)
{
    const char* types[]{
        "Null",
        "Object",
        "Array",
        "String",
        "Integer",
        "Float",
        "Boolean"
    };
    return types[(int)type];
}

void JSONValue::ThrowIfNotType(JSONType type) const {
    if(this->type != type)
        throw JSONException("The contained object is not of type '"s + TypeToString(type) + "'");
}

void JSONValue::Clear()
{
    switch(type)
    {
    case JSONType::Object:
        payload._object.~JSONObject();
        type = JSONType::Null;
        break;
    case JSONType::Array:
        payload._array.~JSONArray();
        type = JSONType::Null;
        break;
    case JSONType::String:
        payload._string.~JSONString();
        type = JSONType::Null;
        break;
    default:
        break;
    }
}

void JSONValue::CopyFrom(const JSONValue& x)
{
    Clear();

    switch(x.type)
    {
    case JSONType::Object:
        new (&payload._object) JSONObject(x.payload._object);
        break;
    case JSONType::Array:
        new (&payload._array) JSONArray(x.payload._array);
        break;
    case JSONType::String:
        new (&payload._string) JSONString(x.payload._string);
        break;
    case JSONType::Integer:
        payload._integer = x.payload._integer;
        break;
    case JSONType::Float:
        payload._float = x.payload._float;
        break;
    case JSONType::Boolean:
        payload._boolean = x.payload._boolean;
        break;
    default:
    case JSONType::Null:
        break;
    }

    type = x.type;
}

void JSONValue::ConstructFrom(JSONValue&& x)
{
    switch(x.type)
    {
    case JSONType::Object:
        new (&payload._object) JSONObject(forward<JSONObject>(x.payload._object));
        break;
    case JSONType::Array:
        new (&payload._array) JSONArray(forward<JSONArray>(x.payload._array));
        break;
    case JSONType::String:
        new (&payload._string) JSONString(forward<JSONString>(x.payload._string));
        break;
    case JSONType::Integer:
        payload._integer = x.payload._integer;
        break;
    case JSONType::Float:
        payload._float = x.payload._float;
        break;
    case JSONType::Boolean:
        payload._boolean = x.payload._boolean;
        break;
    default:
    case JSONType::Null:
        break;
    }

    type = x.type;
    x.type = JSONType::Null;
}

string JSONValue::ToString() const
{
    switch(type)
    {
    default:
    case JSONType::Null:
        return "null";
    case JSONType::Object:
        return "Object";
    case JSONType::Array:
        return "Array";
    case JSONType::String:
        return payload._string;
    case JSONType::Integer:
        return to_string(payload._integer);
    case JSONType::Float:
    {
        char buff[3 + DBL_MANT_DIG - DBL_MIN_EXP];
        sprintf_s(buff, "%g", payload._float);
        return buff;
    }
    case JSONType::Boolean:
        return payload._boolean ? "true" : "false";
    }
}

////////////////////////////////
//    JSONParser
////////////////////////////////

JSONValue JSONParser::Parse(const string& json) {
    if(json.empty()) return JSONValue();
    return ParseValue(Iterator(json.data()));
}

JSONValue JSONParser::Load(const string& filename) {
    ifstream fin(filename);
    if(!fin) throw JSONException("Failed to load file");
    string str((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
    return Parse(str);
}

string JSONParser::Dump(const JSONValue& value, bool pretty) {
    stringstream stream;
    Dump(value, stream, pretty, 0);
    return stream.str();
}

JSONValue JSONParser::ParseValue(Iterator& iter)
{
    switch(*iter)
    {
    case '{': return ParseObject(iter);
    case '[': return ParseArray(iter);
    case '\"': return ParseString(iter);
    case 't':
    case 'f': return ParseBoolean(iter);
    case 'n': return ParseNull(iter);
    default: if(IsNumChar(*iter)) return ParseNumber(iter);
    }

    throw JSONException(iter, "Invalid JSON value.");
}

JSONValue JSONParser::ParseObject(Iterator& iter)
{
    ++iter;

    if(*iter == '}') {
        ++iter;
        return JSONObject();
    }

    JSONObject ret;

    while(true)
    {
        Iterator tmp  = iter;

        JSONValue key = ParseValue(iter);
        if(key.valueType() != JSONType::String)
            throw JSONException(tmp, "Error parsing JSON object. Expected string for element key");

        ++iter;

        if(*iter != ':')
            throw JSONException(iter, "Error parsing JSON object. Expected ':' but found '"s + *iter + "'");

        ++iter;

        JSONValue val = ParseValue(iter);
        ret[key.asString()] = val;
        ++iter;

        if(*iter == ',' )
            ++iter;
        else if(*iter == '}')
            break;
        else
            throw JSONException(iter, "Error parsing JSON object. Expected ',' or '}' but found '"s + *iter + "'");
    }

    return ret;
}

JSONValue JSONParser::ParseArray(Iterator& iter)
{
    ++iter;

    if(*iter == ']') {
        ++iter;
        return JSONArray();
    }

    JSONArray val;

    while(true)
    {
        val.push_back(ParseValue(iter));
        ++iter;

        if(*iter == ',')
            ++iter;
        else if(*iter == ']')
            break;
        else
            throw JSONException(iter, "Error parsing array. Expected ',' or ']' but found '"s + *iter + "'");
    }

    return val;
}

JSONValue JSONParser::ParseString(Iterator& iter)
{
    Iterator tmp = iter + 1;

    while(*tmp != 0 && *tmp != '\"')
    {
        if(*tmp == '\\') {
            tmp += 1;

            switch(*tmp)
            {
            case '\"': case '\\': case '/': case 'b':
            case 'f': case 'n': case 'r': case 't':
                tmp += 1;
                break;

            case 'u':
                tmp += 1;

                for(int i = 0; i < 4; ++i, tmp += 1) {
                    if(!isxdigit(*tmp))
                        throw JSONException(tmp, "Invalid hex character in unicode escape sequence: '"s + *tmp + "'");
                }
                break;

            default:
                throw JSONException(tmp, "Invalid escape sequence: '\\"s + *tmp + "'");
            }
        }
        else {
            tmp += 1;
        }
    }
    
    if(*tmp == 0)
        throw JSONException(iter, "Reached end of input while trying to parse string.");

    iter += 1;

    JSONString ret;
    ret.reserve(tmp.value - iter.value);

    for( ; *iter != '\"'; iter += 1)
    {
        if(*iter == '\\')
        {
            iter += 1;

            switch(*iter)
            {
            case '\"': ret += '\"'; break;
            case '\\': ret += '\\'; break;
            case '/':  ret += '/' ; break;
            case 'b':  ret += '\b'; break;
            case 'f':  ret += '\f'; break;
            case 'n':  ret += '\n'; break;
            case 'r':  ret += '\r'; break;
            case 't':  ret += '\t'; break;
            case 'u':
                ret += "\\u";

                for(unsigned i = 1; i <= 4; ++i)
                    ret += iter[i];

                iter += 4;
                break;
            }
        }
        else
        {
            ret += *iter;
        }
    }

    return ret;
}

JSONValue JSONParser::ParseNumber(Iterator& iter)
{
    const char* val = iter.value;
    const char* val_end = val;
    const char* exp_end = nullptr;

    bool hasDecimal = false;

    while(true)
    {
        if(IsNumChar(*val_end)) {
            ++val_end;
            iter += 1;
        }
        else if(*val_end == '.') {
            ++val_end;
            iter += 1;
            hasDecimal = true;
        }
        else {
            break;
        }
    }

    char c = *val_end;
    if(c == 'e' || c == 'E')
    {
        exp_end = val_end + 1;
        iter += 1;

        if(*exp_end == '-' || *exp_end == '+') {
            ++exp_end;
            iter += 1;
        }
        
        while(true)
        {
            c = *exp_end;

            if(isdigit(c)) {
                ++exp_end;
                iter += 1;
            }
            else if(isspace(c) || c == ',' || c == ']' || c == '}') {
                break;
            }
            else {
                throw JSONException(iter, "Error parsing number. Expected digit but found '"s + c + "'");
            }
        }
    }
    else if(!isspace(c) && c != ',' && c != ']' && c != '}' ) {
        throw JSONException(iter, "Error parsing number. Expected digit but found '"s + c + "'");
    }

    iter -= 1;

    if(hasDecimal) {
        long double num = strtold(val, nullptr);
        if(exp_end && (long long)num == num)
            return (long long)num;
        else
            return (double)num;
    }
    else {
        if(exp_end)
            return strtoll(val, nullptr, 10) * pow(10, strtol(val_end + 1, nullptr, 10));
        else
            return strtoll(val, nullptr, 10);
    }
}

JSONValue JSONParser::ParseBoolean(Iterator& iter)
{
    if(StrCmpI(iter.value, "true", 4)) {
        iter += 3;
        return true;
    }
    else if(StrCmpI(iter.value, "false", 5)) {
        iter += 4;
        return false;
    }

    throw JSONException(iter, "Invalid JSON value: trying to parse boolean");
}

JSONValue JSONParser::ParseNull(Iterator& iter)
{
    if(StrCmpI(iter.value, "null", 4)) {
        iter += 3;
        return nullptr;
    }

    throw JSONException(iter, "Invalid JSON value: trying to parse 'null'");
}

struct JSONIndent
{
    int depth;
    JSONIndent(int depth) : depth(depth){}
    friend ostream& operator<<(ostream& stream, const JSONIndent& indent) {
        for(int i = 0; i < indent.depth; ++i)
            stream << "    ";
        return stream;
    }
};

void JSONParser::Dump(const JSONValue& value, stringstream& output, bool pretty, int depth)
{
    switch(value.valueType())
    {
    default:
    case JSONType::Null:
        output << "null";
        break;

    case JSONType::Object:
    {
        output << "{";
        if(pretty) output << "\n";

        int i = 0;
        for(auto& e : value.asObject())
        {
            if(i++) {
                output << ",";
                if(pretty) output << "\n";
            }

            output << JSONIndent(pretty ? depth + 1 : 0) << "\"" << e.first << "\":";
            if(pretty) output << " ";
            Dump(e.second, output, pretty, depth + 1);
        }

        if(pretty) output << "\n";
        output << JSONIndent(pretty ? depth : 0) << "}";
        break;
    }
    case JSONType::Array:
    {
        output << "[";
        if(pretty) output << "\n";

        int i = 0;
        for(auto& e : value.asArray())
        {
            if(i++) {
                output << ",";
                if(pretty) output << "\n";
            }

            output << JSONIndent(pretty ? depth + 1 : 0);
            Dump(e, output, pretty, depth + 1);
        }

        if(pretty) output << "\n";
        output << JSONIndent(pretty ? depth : 0) << "]";
        break;
    }
    case JSONType::String:
        output << "\"" << value.asString() << "\"";
        break;
    case JSONType::Integer:
        output << value.ToString();
        break;
    case JSONType::Float:
        output << value.ToString();
        break;
    case JSONType::Boolean:
        output << value.ToString();
        break;
    }
}

bool JSONParser::IsNumChar(char c) {
    return (c >= '0' && c <= '9') || c == '-' || c == '+';
}

bool JSONParser::StrCmpI(const char* a, const char* b, size_t n)
{
    for( ; n > 0; --n)
    {
        if(tolower(*a++) != tolower(*b++))
            return false;
    }

    return true;
}

////////////////////////////////
//    JSONParser::Iterator
////////////////////////////////

JSONParser::Iterator::Iterator(const char* json)
    : offset(0), line(0), column(0), value(json) {
    Advance();
}

JSONParser::Iterator& JSONParser::Iterator::operator++() {
    Skip();
    Advance();
    return *this;
}

JSONParser::Iterator JSONParser::Iterator::operator++(int) {
    Iterator ret = *this;
    Skip();
    Advance();
    return ret;
}

JSONParser::Iterator JSONParser::Iterator::operator+(size_t count) const {
    Iterator ret = *this;
    for(size_t i = 0; i < count; ++i) {
        ++ret.offset;
        ++ret.column;
        ++ret.value;
    }
    return ret;
}

JSONParser::Iterator& JSONParser::Iterator::operator+=(size_t count) {
    for(size_t i = 0; i < count; ++i) {
        ++offset;
        ++column;
        ++value;
    }
    return *this;
}

JSONParser::Iterator JSONParser::Iterator::operator-(size_t count) const {
    Iterator ret = *this;
    for(size_t i = 0; i < count; ++i) {
        --ret.offset;
        --ret.column;
        --ret.value;
    }
    return ret;
}

JSONParser::Iterator& JSONParser::Iterator::operator-=(size_t count) {
    for(size_t i = 0; i < count; ++i) {
        --offset;
        --column;
        --value;
    }
    return *this;
}

const char& JSONParser::Iterator::operator*() const {
    return *value;
}

const char& JSONParser::Iterator::operator[](size_t index) const {
    return value[index];
}

string JSONParser::Iterator::location() const {
    char buff[128];
#ifdef _MSC_VER
    sprintf_s(buff, "(%u:%u)", line + 1, column + 1);
#else
    sprintf(buff, "(%u:%u)", line + 1, column + 1);
#endif
    return buff;
}

string JSONParser::Iterator::enclosingLine() const
{
    const char* beg = value - offset;
    const char* line_beg = value;
    const char* line_end = value;

    while(line_beg != beg && *line_beg != '\n')
        --line_beg;

    if(*line_beg == '\n')
        ++line_beg;

    while(*line_end != 0 && *line_end != '\n')
        ++line_end;

    string ret(line_beg, line_end);

    size_t pos = 0;
    while((pos = ret.find('\t', pos)) != string::npos) {
        ret.replace(pos, 1, "    ");
        pos += 4;
    }

    return ret;
}

void JSONParser::Iterator::Skip()
{
    if(!isspace(*value))
    {
        ++column;
        ++value;
    }
}

void JSONParser::Iterator::Advance()
{
    while(*value != 0)
    {
        switch(*value)
        {
        case ' ':
            ++column;
            break;

        case '\t':
            column += 4;
            break;

        case '\n': // new line
            column = 0;
            ++line;
            break;

        case '\v': // vertical tab
        case '\f': // form feed
        case '\r': // carriage return
                   // ignore
            break;

        default:
            goto found_char;
        }

        ++offset;
        ++value;
    }

    if(*value == 0)
        throw JSONException("Reached end of string while searching for token.");

found_char:
    return;
}

////////////////////////////////
//    JSONException
////////////////////////////////

JSONException::JSONException(const std::string& message)
    : msg(message) {

}

JSONException::JSONException(const JSONParser::Iterator& iter, const std::string& message) {
    stringstream stream;
    stream << iter.location() << ": " << message << endl
           << iter.enclosingLine() << endl
           << string(iter.column, ' ') << '^';

    msg = move(stream.str());
}

char const* JSONException::what() const {
    return msg.c_str();
}
