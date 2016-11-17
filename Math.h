/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <cmath>
#include <cstdint>
#include <ostream>

// left handed row-major math library

class Vec3;
class Vec4;
class Mat3;
class Mat4;
class Quat;
class Ray;
class Plane;
class Triangle;
class Sphere;
class Box;

////////////////////////////////
//    Vec2
////////////////////////////////

class Vec2
{
public:
    float x, y;

    static const Vec2 zero;
    static const Vec2 up;
    static const Vec2 right;

    Vec2(){}
    Vec2(float x, float y) : x(x), y(y){}

    Vec2 operator-() const;
    Vec2 operator+(const Vec2& v) const;
    Vec2 operator-(const Vec2& v) const;
    Vec2 operator*(const float s) const;
    Vec2 operator/(const float s) const;
    float operator*(const Vec2& v) const;
    Vec2& operator+=(const Vec2& v);
    Vec2& operator-=(const Vec2& v);
    Vec2& operator*=(const float s);
    Vec2& operator/=(const float s);
    bool operator==(const Vec2& v) const;
    bool operator!=(const Vec2& v) const;
    Vec2 PerpCW() const;
    Vec2 PerpCCW() const;
    Vec2 Scale(const Vec2& v) const;
    float Det(const Vec2& v) const;
    float Dot(const Vec2& v) const;
    float Length() const;
    float LengthSq() const;
    float Distance(const Vec2& v) const;
    float DistanceSq(const Vec2& v) const;
    void Normalize();
    Vec2 Normalized() const;
    bool IsNormalized() const;
};

////////////////////////////////
//    Vec3
////////////////////////////////

class Vec3
{
public:
    float x, y, z;

    static const Vec3 zero;
    static const Vec3 forward;
    static const Vec3 up;
    static const Vec3 right;

    Vec3(){}
    Vec3(float x, float y, float z) : x(x), y(y), z(z){}
    Vec3(const Vec2& v) : x(v.x), y(v.y), z(0){}
    operator Vec2() const { return Vec2(x, y); }

    Vec3 operator-() const;
    Vec3 operator+(const Vec3& v) const;
    Vec3 operator-(const Vec3& v) const;
    Vec3 operator*(float s) const;
    Vec3 operator/(float s) const;
    float operator*(const Vec3& v) const;
    Vec3 operator*(const Mat3& m) const;
    Vec3 operator*(const Quat& q) const;
    Vec3& operator+=(const Vec3& v);
    Vec3& operator-=(const Vec3& v);
    Vec3& operator*=(float s);
    Vec3& operator/=(float s);
    Vec3& operator*=(const Quat& q);
    Vec3& operator*=(const Mat3& m);
    bool operator==(const Vec3& v) const;
    bool operator!=(const Vec3& v) const;
    Vec3 Scale(const Vec3& vec) const;
    float Dot(const Vec3& v) const;
    float Dot(const Plane& p) const;
    Vec3 Cross(const Vec3& v) const;
    float Length() const;
    float LengthSq() const;
    float Distance(const Vec3& v) const;
    float DistanceSq(const Vec3& v) const;
    void Normalize();
    Vec3 Normalized() const;
    bool IsNormalized() const;
    friend std::ostream& operator<<(std::ostream& os, const Vec3& v);
};

////////////////////////////////
//    Vec4
////////////////////////////////

class alignas(16) Vec4
{
public:
    float x, y, z, w;

    static const Vec4 zero;
    static const Vec4 forward;
    static const Vec4 up;
    static const Vec4 right;

    Vec4(){}
    Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w){}
    Vec4(const Vec2& v) : x(v.x), y(v.y), z(0), w(0){}
    Vec4(const Vec2& v, float z, float w) : x(v.x), y(v.y), z(z), w(w){}
    Vec4(const Vec3& v) : x(v.x), y(v.y), z(v.z), w(0){}
    Vec4(const Vec3& v, float w) : x(v.x), y(v.y), z(v.z), w(w){}
    operator Vec2() const { return Vec2(x, y); }
    operator Vec3() const { return Vec3(x, y, z); }
    
    Vec4 operator-() const;
    Vec4 operator+(const Vec4& v) const;
    Vec4 operator-(const Vec4& v) const;
    Vec4 operator*(const float s) const;
    Vec4 operator/(const float s) const;
    float operator*(const Vec4& v) const;
    Vec4 operator*(const Mat4& m) const;
    Vec4& operator+=(const Vec4& v);
    Vec4& operator-=(const Vec4& v);
    Vec4& operator*=(const float s);
    Vec4& operator/=(const float s);
    Vec4& operator*=(const Mat4& m);
    bool operator==(const Vec4& v) const;
    bool operator!=(const Vec4& v) const;
    Vec4 Scale(const Vec4& v) const;
    float Dot(const Vec4& v) const;
    float Dot(const Plane& p) const;
    float Distance(const Vec4& v) const;
    float DistanceSq(const Vec4& v) const;
    float Length() const;
    float LengthSq() const;
    void Normalize();
    Vec4 Normalized() const;
    bool IsNormalized() const;
};

////////////////////////////////
//    Mat3
////////////////////////////////

class Mat3
{
public:
    static const Mat3 zero;
    static const Mat3 identity;
    
    float m11, m12, m13, // row 1
          m21, m22, m23, // row 2
          m31, m32, m33; // row 3

    Mat3(){}
    Mat3(float m11, float m12, float m13,
         float m21, float m22, float m23,
         float m31, float m32, float m33)
       :m11(m11), m12(m12), m13(m13),
        m21(m21), m22(m22), m23(m23),
        m31(m31), m32(m32), m33(m33){}

    Mat3 operator+(const Mat3& m) const;
    Mat3 operator-(const Mat3& m) const;
    Mat3 operator+(float s) const;
    Mat3 operator-(float s) const;
    Mat3 operator*(float s) const;
    Mat3 operator/(float s) const;
    Mat3 operator*(const Mat3& m) const;
    Mat3& operator+=(const Mat3& m);
    Mat3& operator-=(const Mat3& m);
    Mat3& operator+=(float s);
    Mat3& operator-=(float s);
    Mat3& operator*=(float s);
    Mat3& operator/=(float s);
    Mat3& operator*=(const Mat3& m);
    bool operator==(const Mat3& m) const;
    bool operator!=(const Mat3& m) const;

    void Transpose();
    Mat3 Transposed() const;
    Mat3 Invert();
    Mat3 Inverse() const;
    Quat GetRotation() const;

    static Mat3 Scale(float x, float y, float z);
    static Mat3 Translation(float x, float y);
    static Mat3 Ortho2D(float left, float right, float bottom, float top);
    static Mat3 LookRotation(const Vec3& forward, const Vec3& upward);
};

////////////////////////////////
//    Mat4
////////////////////////////////

class alignas(16) Mat4
{
public:
    static const Mat4 zero;
    static const Mat4 identity;
    
    float m11, m12, m13, m14, //row 1
          m21, m22, m23, m24, //row 2
          m31, m32, m33, m34, //row 3
          m41, m42, m43, m44; //row 4
    
    Mat4(){}
    Mat4(float m11, float m12, float m13, float m14,
         float m21, float m22, float m23, float m24,
         float m31, float m32, float m33, float m34,
         float m41, float m42, float m43, float m44)
         :m11(m11), m12(m12), m13(m13), m14(m14),
          m21(m21), m22(m22), m23(m23), m24(m24),
          m31(m31), m32(m32), m33(m33), m34(m34),
          m41(m41), m42(m42), m43(m43), m44(m44){}
    Mat4(const Mat3& m)
        :m11(m.m11), m21(m.m21), m31(m.m31), m41(0),
         m12(m.m12), m22(m.m22), m32(m.m32), m42(0),
         m13(m.m13), m23(m.m23), m33(m.m33), m43(0),
         m14(0),     m24(0),     m34(0),     m44(1){}

    Mat4 operator+(const Mat4& m) const;
    Mat4 operator-(const Mat4& m) const;
    Mat4 operator+(float s) const;
    Mat4 operator-(float s) const;
    Mat4 operator*(float s) const;
    Mat4 operator/(float s) const;
    Mat4 operator*(const Mat4& m) const;
    Mat4& operator+=(const Mat4& m);
    Mat4& operator-=(const Mat4& m);
    Mat4& operator+=(float s);
    Mat4& operator-=(float s);
    Mat4& operator*=(float s);
    Mat4& operator/=(float s);
    Mat4& operator*=(const Mat4& m);
    bool operator==(const Mat4& m) const;
    bool operator!=(const Mat4& m) const;

    void Transpose();
    Mat4 Transposed() const;
    Mat4 Invert();
    Mat4 Inverse() const;
    //Quat GetRotation() const;

    static Mat4 Scale(float x, float y, float z);
    static Mat4 Translation(float x, float y, float z);
    static Mat4 Rotation(float x, float y, float z);
    static Mat4 XRotation(float angle);
    static Mat4 YRotation(float angle);
    static Mat4 ZRotation(float angle);
    static Mat4 LookRotation(const Vec3& forward, const Vec3& upward);
    static Mat4 Transform(const Vec3& pos, const Vec3& scale, const Quat& rot);
    static Mat4 InverseTransform(const Vec3& pos, const Vec3& scale, const Quat& rot);
    static Mat4 Ortho2D(float left, float right, float bottom, float top, float zNear, float zFar);
    static Mat4 Project3D(float fov, float width, float height, float near, float far);
    static Mat4 Project3D(float fov, float aspect, float near, float far);
};

////////////////////////////////
//    Quat
////////////////////////////////

class alignas(16) Quat
{
public:
    Vec3 v;
    float w;

    static const Quat zero;
    static const Quat identity;
    
    Quat(){}
    Quat(float x, float y, float z, float w);
    Quat(const Vec3& v, float w);
    Quat(float xAngle, float yAngle, float zAngle);
    Quat(const Vec3& angles);

    Quat operator+(const Quat& q) const;
    Quat operator-(const Quat& q) const;
    Quat operator*(const Quat& q) const;
    Quat operator/(const Quat& q) const;
    Quat operator*(float s) const;
    Quat operator/(float s) const;
    Quat operator-() const;
    const Quat& operator+=(const Quat& q);
    const Quat& operator-=(const Quat& q);
    const Quat& operator*=(const Quat& q);
    const Quat& operator*=(float s);
    const Quat& operator/=(float s);
    float Dot(const Quat& q) const;
    Mat3 ToMatrix() const;

    float Length() const;
    float LengthSq() const;
    void Normalize();
    Quat Normalized() const;
    void Invert();
    Quat Inverse() const;

    static Quat AngleAxis(float angle, const Vec3& axis);
    static Quat FromTo(const Vec3& from, const Vec3& to);
    static Quat LookRotation(const Vec3& forward, const Vec3& upward);
    static Quat Lerp(const Quat& a, const Quat& b, float t);
    static Quat Slerp(const Quat& a, const Quat& b, float t);

    void ToAngleAxis(float& angle, Vec3& axis) const;
    Vec3 ToEulerAngles() const;
};

////////////////////////////////
//    Ray
////////////////////////////////

class Ray
{
public:
    Vec3 origin;
    Vec3 direction;

    Ray(){}
    Ray(const Vec3& orig, const Vec3& dir)
        : origin(orig), direction(dir){}

    bool Cast(const Plane& p) const;
    bool Cast(const Plane& p, Vec3& hitpoint) const;
    bool Cast(const Sphere& s) const;
    bool Cast(const Sphere& s, Vec3& hitpoint) const;
    bool Cast(const Triangle& tri) const;
    bool Cast(const Triangle& tri, Vec3& hitpoint) const;
    bool CastNoCull(const Triangle& tri) const; // no backface culling
    bool CastNoCull(const Triangle& tri, Vec3& hitpoint) const;
};

////////////////////////////////
//    Plane
////////////////////////////////

class Plane
{
public:
    float a, b, c, d;

    Plane(){}
    Plane(float a, float b, float c, float d)
        : a(a), b(b), c(c), d(d){}
    
    void Normalize();
    Vec3 Normal() const;
    bool InFront(const Vec3& point) const;
    bool InBack(const Vec3& point) const;
    bool InFront(const Sphere& sphere) const;
    bool InBack(const Sphere& sphere) const;
    operator Vec4() const;
};

////////////////////////////////
//    Triangle
////////////////////////////////

class Triangle
{
public:
    Vec3 a;
    Vec3 b;
    Vec3 c;

    Triangle(){}
    Triangle(const Vec3& a, const Vec3& b, const Vec3& c)
        : a(a), b(b), c(c) {}
};

////////////////////////////////
//    Sphere
////////////////////////////////

class Sphere
{
public:
    Vec3 center;
    float radius;

    Sphere(){}
    Sphere(const Vec3& center, float radius)
        : center(center), radius(radius) {}

    Sphere(float x, float y, float z, float radius)
        : center(x, y, z), radius(radius) {}

    Sphere operator+(const Sphere& other);
    Sphere& operator+=(const Sphere& other);
};

////////////////////////////////
//    Box
////////////////////////////////

class Box
{
public:
    Vec3 vmin;
    Vec3 vmax;

    Box(){}
    Box(const Vec3& vmin, const Vec3& vmax) 
        : vmin(vmin), vmax(vmax){}

    void GetVerts(Vec3 (&verts)[8]) const;
    Box operator+(const Box& other);
    Box& operator+=(const Box& other);
};

////////////////////////////////
//    Rect
////////////////////////////////

class Rect
{
public:
    int x;
    int y;
    int w;
    int h;

    Rect(){}
    Rect(int x, int y, int w, int h)
        : x(x), y(y), w(w), h(h){}

    void FitInto(const Rect& rc)
    {
        float wx = (float)rc.x;
        float wy = (float)rc.y;
        float ww = (float)rc.w;
        float wh = (float)rc.h;

        float s = 1.0f;

        if(w > h)
        {
            s = wh / (float)h;

            if((float)w * s > ww)
                s = ww / (float)w;
        }
        else
        {
            s = ww / (float)w;

            if((float)h * s > wh)
                s = wh / (float)h;
        }

        float sw = (float)w * s;
        float sh = (float)h * s;

        x = wx + (sw < ww - 0.5f) ? (int)((ww - sw) * 0.5f) : 0;
        y = ww + (sh < wh - 0.5f) ? (int)((wh - sh) * 0.5f) : 0;
        w = (int)sw;
        h = (int)sh;
    }
};

////////////////////////////////
//    ColorBGRA
////////////////////////////////

class alignas(4) ColorBGRA
{
public:
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t a;

    ColorBGRA(){}
    ColorBGRA(uint8_t b, uint8_t g, uint8_t r, uint8_t a)
        : b(b), g(g), r(r), a(a){}

    ColorBGRA(uint32_t c);
    operator uint32_t() const;
};

////////////////////////////////
//    Color32
////////////////////////////////

class Color32
{
public:
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;

    Color32(){}
    Color32(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
        : r(r), g(g), b(b), a(a){}

    Color32(uint32_t c);
    Color32(ColorBGRA c);
    operator uint32_t() const;
    operator ColorBGRA() const;
};

////////////////////////////////
//    Color
////////////////////////////////

class alignas(16) Color
{
public:
    float r;
    float g;
    float b;
    float a;

    static const Color red;
    static const Color green;
    static const Color blue;
    static const Color yellow;
    static const Color orange;
    static const Color purple;
    static const Color magenta;
    static const Color cyan;
    static const Color black;
    static const Color white;
    static const Color gray;
    static const Color clear;

    Color(){}
    Color(float r, float g, float b, float a)
        : r(r), g(g), b(b), a(a){}
    Color(const Vec4& v)
        : r(v.x), g(v.y), b(v.z), a(v.w){}

    Color(Color32 c);
    Color(uint32_t c);

    Color& operator=(const Vec4& v);
    Color operator+(const Color& c) const;
    Color operator-(const Color& c) const;
    Color operator*(float s) const;
    Color operator*(const Color& c) const;
    Color& operator*=(float s);
    Color& operator*=(const Color& c);
    Color& operator+=(const Color& c);
    Color Blend(const Color& dst) const;
    static Color Lerp(const Color& a, const Color& b, float t);
    static Color Clamp(const Color& c, float lower = 0.0f, float upper = 1.0f);
    operator uint32_t();
    operator Color32();
    operator Vec4&();
    operator float*();
};


////////////////////////////////
//    Random
////////////////////////////////

class Random
{
public:
    static float value();
    static float signedValue();
    static Vec2 vector2();
    static Vec3 vector3();
    static float range(float min, float max);
};

////////////////////////////////
//    Misc
////////////////////////////////


std::ostream& operator<<(std::ostream& os, const Vec4& v);
std::ostream& operator<<(std::ostream& os, const Mat3& m);
std::ostream& operator<<(std::ostream& os, const Mat4& m);
std::ostream& operator<<(std::ostream& os, const Quat& q);

namespace Math
{
    constexpr float Pi = 3.141592654f;
    constexpr float HalfPi = Pi / 2.0f;
    constexpr float TwoPi = Pi * 2.0f;
    constexpr float DegToRad = Pi / 180.0f;
    constexpr float RadToDeg = 180.0f / Pi;
    constexpr float FloatTolerance = FLT_EPSILON * 3;

    inline float ToRadians(float degrees) {
        return degrees * DegToRad;
    }

    inline float ToDegrees(float radians) {
        return radians * RadToDeg;
    }

    inline int Floor(float x) {
        int i = (int)x;
        return i - (x < i);
    }

    inline int Ceil(float x) {
        int i = (int)x;
        return i + (x > i);
    }

    template<bool is_ieee754 = std::numeric_limits<float>::is_iec559>
    inline float Log2(float val) {
        union { float val; int32_t x; } u = { val };
        float ret = (float)(((u.x >> 23) & 255) - 128);
        u.x &= ~(255 << 23);
        u.x += 127 << 23;
        ret += ((-0.3358287811f) * u.val + 2.0f) * u.val  -0.65871759316667f;
        return ret;
    }

    template<>
    inline float Log2<false>(float val) {
        return log2(val);
    }

    inline float Acos(float x) {
        return (-0.6981316805f * x * x - 0.8726646304f) * x + 1.570796371f;
    }

    template<typename T>
    inline T Min(T a) {
        return std::forward<T>(a);
    }

    template<typename T, typename... Ts>
    inline T Min(T a, T b, Ts... ts) {
        return Min((a < b) ? a : b, ts...);
    }

    template<typename T>
    inline T Max(T a) {
        return a;
    }

    template<typename T, typename... Ts>
    inline T Max(T a, T b, Ts... ts) {
        return Max((a > b) ? a : b, ts...);
    }

    template<typename T>
    inline T Clamp(T x, T lower, T upper) {
        return (x < lower) ? lower : ((x > upper) ? upper : x);
    }

    inline float Clamp01(float value) {
        float tmp = value + 1.0f - abs(value - 1.0f);
        return (tmp + abs(tmp)) * 0.25f;
    }

    template<typename T>
    inline T NormalizedClamp(T x, T lower, T upper) {
        return Clamp01((x - lower) / (upper - lower));
    }

    inline float Snap(float n, float span)
    {
        float ret;

        if(n < 0)
        {
            ret = -n + (0.5f * span);
            ret = -ret + fmod(ret, span);
        }
        else
        {
            ret = n + (0.5f * span);
            ret = ret - fmod(ret, span);
        }

        return ret;
    }

    template<class T>
    inline T Loop(T a, T Min, T Max)
    {
        T range = Max - Min;
        while(a > Max) a -= range;
        while(a < Min) a += range;
        return a;
    }

    template<typename T, typename M>
    inline T Lerp(const T& a, const T& b, M t) {
        return a + (b - a) * t;
    }
    
    inline int NextPowerOfTwo(int num)
    {
        int ret = 1;

        while(ret < num)
            ret <<= 1;

        return ret;
    }

    inline bool IsPowerOfTwo(unsigned int num) {
        return (num != 0) && ((num & (num - 1)) == 0);
    }

    inline Vec3 CalcBarycentricCoords(const Vec2& a, const Vec2& b, const Vec2& c, const Vec2& p)
    {
        Vec2 e0 = b - a;
        Vec2 e1 = c - a;
        Vec2 e2 = p - a;

        float d00 = e0.Dot(e0);
        float d01 = e0.Dot(e1);
        float d11 = e1.Dot(e1);
        float d20 = e2.Dot(e0);
        float d21 = e2.Dot(e1);
        float denom = d00 * d11 - d01 * d01;
        float v = (d11 * d20 - d01 * d21) / denom;
        float w = (d00 * d21 - d01 * d20) / denom;
        float u = 1.0f - v - w;

        return Vec3(u, v, w);
    }
}
