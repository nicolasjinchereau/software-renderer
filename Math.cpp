/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#include "Math.h"
#include "SIMD.h"
#include <algorithm>
#include <cassert>
using namespace Math;
using namespace std;

////////////////////////////////
//    Vec2
////////////////////////////////

const Vec2 Vec2::zero(0, 0);
const Vec2 Vec2::up(0, 1);
const Vec2 Vec2::right(1, 0);

Vec2 Vec2::operator-() const {
    return Vec2(-x, -y);
}

Vec2 Vec2::operator+(const Vec2& v) const {
    return Vec2(x + v.x, y + v.y);
}

Vec2 Vec2::operator-(const Vec2& v) const {
    return Vec2(x - v.x, y - v.y);
}

Vec2 Vec2::operator*(const float s) const {
    return Vec2(x * s, y * s);
}

Vec2 Vec2::operator/(const float s) const {
    return Vec2(x / s, y / s);
}

float Vec2::operator*(const Vec2& v) const {
    return x * v.x + y * v.y;
}

Vec2& Vec2::operator+=(const Vec2& v)
{
    x += v.x;
    y += v.y;
    return *this;
}

Vec2& Vec2::operator-=(const Vec2& v)
{
    x -= v.x;
    y -= v.y;
    return *this;
}

Vec2& Vec2::operator*=(const float s)
{
    x *= s;
    y *= s;
    return *this;
}

Vec2& Vec2::operator/=(const float s)
{
    x /= s;
    y /= s;
    return *this;
}

bool Vec2::operator==(const Vec2& v) const {
    return (x == v.x && y == v.y);
}

bool Vec2::operator!=(const Vec2& v) const {
    return (x != v.x || y != v.y);
}

Vec2 Vec2::PerpCW() const {
    return Vec2(y, -x);
}

Vec2 Vec2::PerpCCW() const {
    return Vec2(-y, x);
}

Vec2 Vec2::Scale(const Vec2& v) const {
    return Vec2(x * v.x, y * v.y);
}

float Vec2::Det(const Vec2& v) const {
    return x * v.y - y * v.x;
}

float Vec2::Dot(const Vec2& v) const {
    return x * v.x + y * v.y;
}

float Vec2::Length() const {
    return sqrt(x * x + y * y);
}

float Vec2::LengthSq() const {
    return x * x + y * y;
}

float Vec2::Distance(const Vec2& v) const {
    return Vec2(v.x - x, v.y - y).Length();
}

float Vec2::DistanceSq(const Vec2& v) const {
    return Vec2(v.x - x, v.y - y).LengthSq();
}

void Vec2::Normalize()
{
    float sqLen = x * x + y * y;
    if(sqLen >= FloatTolerance && abs(1.0f - sqLen) > FloatTolerance)
        *this /= sqrt(sqLen);
}

Vec2 Vec2::Normalized() const
{
    float sqLen = x * x + y * y;
    return sqLen >= FloatTolerance && abs(1.0f - sqLen) > FloatTolerance ?
        *this / sqrt(sqLen) : *this;
}

bool Vec2::IsNormalized() const
{
    float sqLen = x * x + y * y;
    return abs(1.0f - sqLen) <= FloatTolerance;
}

////////////////////////////////
//    Vec3
////////////////////////////////

const Vec3 Vec3::zero(0, 0, 0);
const Vec3 Vec3::forward(0, 0, 1);
const Vec3 Vec3::up(0, 1, 0);
const Vec3 Vec3::right(1, 0, 0);

Vec3 Vec3::operator-() const {
    return Vec3(-x, -y, -z);
}

Vec3 Vec3::operator+(const Vec3& v) const {
    return Vec3(x + v.x, y + v.y, z + v.z);
}

Vec3 Vec3::operator-(const Vec3& v) const {
    return Vec3(x - v.x, y - v.y, z - v.z);
}

Vec3 Vec3::operator*(float s) const {
    return Vec3(x * s, y * s, z * s);
}

Vec3 Vec3::operator/(float s) const {
    return Vec3(x / s, y / s, z / s);
}

float Vec3::operator*(const Vec3& v) const {
    return (x * v.x) + (y * v.y) + (z * v.z);
}

Vec3 Vec3::operator*(const Quat& q) const
{
    Vec3 uv = q.v.Cross(*this);
    Vec3 uuv = q.v.Cross(uv);
    uv = uv * (2.0f * q.w);
    uuv = uuv * 2.0f;
    return *this + uv + uuv;
}

Vec3 Vec3::operator*(const Mat3& m) const
{
    return Vec3(
        x * m.m11 + y * m.m21 + z * m.m31,
        x * m.m12 + y * m.m22 + z * m.m32,
        x * m.m13 + y * m.m23 + z * m.m33);
}

Vec3& Vec3::operator+=(const Vec3& v) {
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
}

Vec3& Vec3::operator-=(const Vec3& v) {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
}

Vec3& Vec3::operator*=(float s) {
    x *= s;
    y *= s;
    z *= s;
    return *this;
}

Vec3& Vec3::operator/=(float s) {
    x /= s;
    y /= s;
    z /= s;
    return *this;
}

Vec3& Vec3::operator*=(const Quat& q) {
    return (*this = *this * q);
}

Vec3& Vec3::operator*=(const Mat3& m) {
    return *this = *this * m;
}

bool Vec3::operator==(const Vec3& v) const {
    return (x == v.x && y == v.y && z == v.z);
}

bool Vec3::operator!=(const Vec3& v) const {
    return (x != v.x || y != v.y || z != v.z);
}

Vec3 Vec3::Scale(const Vec3& v) const {
    return Vec3(x * v.x, y * v.y, z * v.z);
}

float Vec3::Dot(const Vec3 & v) const {
    return x * v.x + y * v.y + z * v.z;
}

float Vec3::Dot(const Plane& p) const {
    return x * p.a + y * p.b + z * p.c + p.d;
}

Vec3 Vec3::Cross(const Vec3 & v) const
{
    return Vec3((y * v.z) - (z * v.y),
                (z * v.x) - (x * v.z),
                (x * v.y) - (y * v.x));
}

float Vec3::Length() const {
    return sqrt(x * x + y * y + z * z);
}

float Vec3::LengthSq() const {
    return x * x + y * y + z * z;
}

float Vec3::Distance(const Vec3& v) const {
    return Vec3(v.x - x, v.y - y, v.z - z).Length();
}

float Vec3::DistanceSq(const Vec3& v) const {
    return Vec3(v.x - x, v.y - y, v.z - z).LengthSq();
}

void Vec3::Normalize()
{
    float sqLen = x * x + y * y + z * z;
    if(sqLen >= FloatTolerance && abs(1.0f - sqLen) > FloatTolerance)
        *this /= sqrt(sqLen);
}

Vec3 Vec3::Normalized() const
{
    float sqLen = x * x + y * y + z * z;
    return sqLen >= FloatTolerance && abs(1.0f - sqLen) > FloatTolerance ?
        *this / sqrt(sqLen) : *this;
}

bool Vec3::IsNormalized() const {
    float sqLen = x * x + y * y + z * z;
    return abs(1.0f - sqLen) <= FloatTolerance;
}

std::ostream& operator<<(std::ostream& os, const Vec3& v) {
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}

////////////////////////////////
//    Vec4
////////////////////////////////

const Vec4 Vec4::zero(0, 0, 0, 0);
const Vec4 Vec4::forward(0, 0, 1, 0);
const Vec4 Vec4::up(0, 1, 0, 0);
const Vec4 Vec4::right(1, 0, 0, 0);

Vec4 Vec4::operator-() const {
    return Vec4(-x, -y, -z, -w);
}

Vec4 Vec4::operator+(const Vec4& v) const
{
#if USE_SSE
    Vec4 ret;
    vadd(&x, &v.x, &ret.x);
    return ret;
#else
    return Vec4(x + v.x, y + v.y, z + v.z, w + v.w);
#endif
}

Vec4 Vec4::operator-(const Vec4& v) const
{
#if USE_SSE
    Vec4 ret;
    vsub(&x, &v.x, &ret.x);
    return ret;
#else
    return Vec4(x - v.x, y - v.y, z - v.z, w - v.w);
#endif
}

Vec4 Vec4::operator*(const float s) const
{
#if USE_SSE
    Vec4 ret;
    vmul(&x, s, &ret.x);
    return ret;
#else
    return Vec4(x * s, y * s, z * s, w * s);
#endif
}

Vec4 Vec4::operator/(const float s) const
{
#if USE_SSE
    Vec4 ret;
    vdiv(&x, s, &ret.x);
    return ret;
#else
    return Vec4(x / s, y / s, z / s, w / s);
#endif
}

float Vec4::operator*(const Vec4& v) const {
    return x * v.x + y * v.y + z * v.z + w * v.w;
}

Vec4 Vec4::operator*(const Mat4& m) const
{
#if USE_SSE
	Vec4 ret;
    vmulm(&x, &m.m11, &ret.x);
	return ret;
#else
    return Vec4(
        m.m11 * x + m.m21 * y + m.m31 * z + m.m41 * w,
        m.m12 * x + m.m22 * y + m.m32 * z + m.m42 * w,
        m.m13 * x + m.m23 * y + m.m33 * z + m.m43 * w,
        m.m14 * x + m.m24 * y + m.m34 * z + m.m44 * w);
#endif
}

Vec4& Vec4::operator+=(const Vec4& v)
{
#if USE_SSE
    vadd(&x, &v.x, &x);
#else
    x += v.x;
    y += v.y;
    z += v.z;
    w += v.w;
#endif
    return *this;
}

Vec4& Vec4::operator-=(const Vec4& v)
{
#if USE_SSE
    vsub(&x, &v.x, &x);
#else
    x -= v.x;
    y -= v.y;
    z -= v.z;
    w -= v.w;
#endif
    return *this;
}

Vec4& Vec4::operator*=(const float s)
{
#if USE_SSE
    vmul(&x, s, &x);
#else
    x *= s;
    y *= s;
    z *= s;
    w *= s;
#endif
    return *this;
}

Vec4& Vec4::operator/=(const float s)
{
#if USE_SSE
    vdiv(&x, s, &x);
#else
    float rs = 1.0f / s;
    x *= rs;
    y *= rs;
    z *= rs;
    w *= rs;
#endif
    return *this;
}

Vec4& Vec4::operator*=(const Mat4& m) {
    return (*this = *this * m);
}

bool Vec4::operator==(const Vec4& v) const {
    return (x == v.x && y == v.y && z == v.z && v.w == w);
}

bool Vec4::operator!=(const Vec4& v) const {
    return (x != v.x || y != v.y || z != v.z || w != v.w);
}

Vec4 Vec4::Scale(const Vec4& v) const
{
#if USE_SSE
    Vec4 ret;
    vmul(&x, &v.x, &ret.x);
    return ret;
#else
    return Vec4(x * v.x, y * v.y, z * v.z, w * v.w);
#endif
}

float Vec4::Dot(const Vec4& v) const {
    return x * v.x + y * v.y + z * v.z + w * v.w;
}

float Vec4::Dot(const Plane& p) const {
    return x * p.a + y * p.b + z * p.c + w * p.d;
}

float Vec4::Distance(const Vec4& v) const {
    return Vec4(v.x - x, v.y - y, v.z - z, v.w - w).Length();
}

float Vec4::DistanceSq(const Vec4& v) const {
    return Vec4(v.x - x, v.y - y, v.z - z, v.w - w).LengthSq();
}

float Vec4::Length() const {
    return sqrt(x * x + y * y + z * z + w * w);
}

float Vec4::LengthSq() const {
    return x * x + y * y + z * z + w * w;
}

void Vec4::Normalize()
{
    float sqLen = x * x + y * y + z * z + w * w;
    if(sqLen >= FloatTolerance && abs(1.0f - sqLen) > FloatTolerance)
        *this /= sqrt(sqLen);
}

Vec4 Vec4::Normalized() const
{
    float sqLen = x * x + y * y + z * z + w * w;
    return sqLen >= FloatTolerance && abs(1.0f - sqLen) > FloatTolerance ?
        *this / sqrt(sqLen) : *this;
}

bool Vec4::IsNormalized() const
{
    float sqLen = x * x + y * y + z * z + w * w;
    return abs(1.0f - sqLen) <= FloatTolerance;
}

std::ostream& operator<<(std::ostream& os, const Vec4& v) {
    os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
    return os;
}

////////////////////////////////
//    Mat3
////////////////////////////////

const Mat3 Mat3::zero(
    0, 0, 0,    
    0, 0, 0,
    0, 0, 0);

const Mat3 Mat3::identity(
    1, 0, 0,    
    0, 1, 0,
    0, 0, 1);

Mat3 Mat3::operator+(const Mat3& m) const
{
    return Mat3(
        m11 + m.m11, m12 + m.m12, m13 + m.m13,
        m21 + m.m21, m22 + m.m22, m23 + m.m23,
        m31 + m.m31, m32 + m.m32, m33 + m.m33);
}

Mat3 Mat3::operator-(const Mat3& m) const
{
    return Mat3(
        m11 - m.m11, m12 - m.m12, m13 - m.m13,
        m21 - m.m21, m22 - m.m22, m23 - m.m23,
        m31 - m.m31, m32 - m.m32, m33 - m.m33);
}

Mat3 Mat3::operator+(float s) const
{
    return Mat3(
        m11 + s, m12 + s, m13 + s,
        m21 + s, m22 + s, m23 + s,
        m31 + s, m32 + s, m33 + s);
}

Mat3 Mat3::operator-(float s) const
{
    return Mat3(
        m11 - s, m12 - s, m13 - s,
        m21 - s, m22 - s, m23 - s,
        m31 - s, m32 - s, m33 - s);
}

Mat3 Mat3::operator*(float s) const
{
    return Mat3(
        m11 * s, m12 * s, m13 * s,
        m21 * s, m22 * s, m23 * s,
        m31 * s, m32 * s, m33 * s);
}

Mat3 Mat3::operator/(float s) const
{
    float ss = 1.0f / s;
    return Mat3(
        m11 * ss, m12 * ss, m13 * ss,
        m21 * ss, m22 * ss, m23 * ss,
        m31 * ss, m32 * ss, m33 * ss);
}

Mat3 Mat3::operator*(const Mat3& m) const
{
    return Mat3(
        m11 * m.m11 + m12 * m.m21 + m13 * m.m31,
        m11 * m.m12 + m12 * m.m22 + m13 * m.m32,
        m11 * m.m13 + m12 * m.m23 + m13 * m.m33,

        m21 * m.m11 + m22 * m.m21 + m23 * m.m31,
        m21 * m.m12 + m22 * m.m22 + m23 * m.m32,
        m21 * m.m13 + m22 * m.m23 + m23 * m.m33,

        m31 * m.m11 + m32 * m.m21 + m33 * m.m31,
        m31 * m.m12 + m32 * m.m22 + m33 * m.m32,
        m31 * m.m13 + m32 * m.m23 + m33 * m.m33);
}

Mat3& Mat3::operator+=(const Mat3& m) {
    return (*this = *this + m);
}

Mat3& Mat3::operator-=(const Mat3& m) {
    return (*this = *this - m);
}

Mat3& Mat3::operator+=(float s) {
    return (*this = *this + s);
}

Mat3& Mat3::operator-=(float s) {
    return (*this = *this - s);
}

Mat3& Mat3::operator*=(float s){
    return (*this = *this * s);
}

Mat3& Mat3::operator/=(float s){
    return (*this = *this / s);
}

Mat3& Mat3::operator*=(const Mat3& m){
    return (*this = *this * m);
}

bool Mat3::operator==(const Mat3& other) const
{
    return
        m11 == other.m11 && m12 == other.m12 && m13 == other.m13 &&
        m21 == other.m21 && m22 == other.m22 && m23 == other.m23 &&
        m31 == other.m31 && m32 == other.m32 && m33 == other.m33;
}

bool Mat3::operator!=(const Mat3& other) const
{
    return
        m11 != other.m11 || m12 != other.m12 || m13 != other.m13 ||
        m21 != other.m21 || m22 != other.m22 || m23 != other.m23 ||
        m31 != other.m31 || m32 != other.m32 || m33 != other.m33;
}

void Mat3::Transpose() {
    swap(m12, m21);
    swap(m13, m31);
    swap(m23, m32);
}

Mat3 Mat3::Transposed() const {
    return Mat3(
        m11, m21, m31,
        m12, m22, m32,
        m13, m23, m33);
}

Mat3 Mat3::Invert() {
    return (*this = Inverse());
}

Mat3 Mat3::Inverse() const
{
    Mat3 adj(
        (m22 * m33 - m23 * m32), -(m12 * m33 - m13 * m32),  (m12 * m23 - m13 * m22),
       -(m21 * m33 - m23 * m31),  (m11 * m33 - m13 * m31), -(m11 * m23 - m13 * m21),
        (m21 * m32 - m22 * m31), -(m11 * m32 - m12 * m31),  (m11 * m22 - m12 * m21)
        );

    float det = m11 * adj.m11 + m12 * adj.m21 + m13 * adj.m31;
    if(abs(det) < FLT_EPSILON)
        return *this;

    return adj * (1.0f / det);
}

Quat Mat3::GetRotation() const
{
    float trace = 1.0f + m11 + m22 + m33;

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;

    if(trace > 0.000001f)
    {
        float s = sqrt(trace) * 2.0f;
        float inv_s = 1.0f / s;

        x = (m23 - m32) * inv_s;
        y = (m31 - m13) * inv_s;
        z = (m12 - m21) * inv_s;
        w = 0.25f * s;
    }
    else
    {
        if(m11 > m22 && m11 > m33)
        {
            // Column 0:
            float s = sqrt(1.0f + m11 - m22 - m33) * 2.0f;
            float inv_s = 1.0f / s;

            x = 0.25f * s;
            y = (m12 + m21) * inv_s;
            z = (m31 + m13) * inv_s;
            w = (m23 - m32) * inv_s;
        }
        else if(m22 > m33)
        {
            // Column 1:
            float s = sqrt(1.0f + m22 - m11 - m33) * 2.0f;
            float inv_s = 1.0f / s;

            x = (m12 + m21) * inv_s;
            y = 0.25f * s;
            z = (m23 + m32) * inv_s;
            w = (m31 - m13) * inv_s;
        }
        else
        {
            // Column 2:
            float s = sqrt(1.0f + m33 - m11 - m22) * 2.0f;
            float inv_s = 1.0f / s;

            x = (m31 + m13) * inv_s;
            y = (m23 + m32) * inv_s;
            z = 0.25f * s;
            w = (m12 - m21) * inv_s;
        }
    }

    return Quat(x, y, z, w).Normalized();
}

Mat3 Mat3::Scale(float x, float y, float z)
{
    return Mat3(
        x, 0, 0,
        0, y, 0,
        0, 0, z);
}

Mat3 Mat3::Translation(float x, float y)
{
    return Mat3(
        1, 0, 0,
        0, 1, 0,
        x, y, 1);
}

Mat3 Mat3::Ortho2D(float left, float right, float bottom, float top)
{
    float w = right - left;
    float h = top - bottom;

    if(abs(w) < Math::FloatTolerance
    || abs(h) < Math::FloatTolerance)
        return Mat3::identity;

    float sx = 2.0f / w;
    float sy = 2.0f / h;
    float tx = -(left + right) / w;
    float ty = -(top + bottom) / h;
    
    return Mat3(sx, 0,  0,
                0,  sy, 0,
                tx, ty, 1);
}

Mat3 Mat3::LookRotation(const Vec3& forward, const Vec3& upward)
{
    Vec3 zaxis = forward.Normalized();
    Vec3 xaxis = upward.Cross(zaxis).Normalized();
    Vec3 yaxis = zaxis.Cross(xaxis);
    
    return Mat3(xaxis.x, xaxis.y, xaxis.z,
                yaxis.x, yaxis.y, yaxis.z,
                zaxis.x, zaxis.y, zaxis.z);
}

std::ostream& operator<<(std::ostream& os, const Mat3& m) {
    os << "[" << m.m11 << ", " << m.m12 << ", " << m.m13 << "]" << std::endl;
    os << "[" << m.m21 << ", " << m.m22 << ", " << m.m23 << "]" << std::endl;
    os << "[" << m.m31 << ", " << m.m32 << ", " << m.m33 << "]";
    return os;
}

////////////////////////////////
//    Mat4
////////////////////////////////

const Mat4 Mat4::zero(
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0);

const Mat4 Mat4::identity(
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1);

Mat4 Mat4::operator+(const Mat4& m) const
{
    return Mat4(
        m11 + m.m11, m12 + m.m12, m13 + m.m13, m14 + m.m14,
        m21 + m.m21, m22 + m.m22, m23 + m.m23, m24 + m.m24,
        m31 + m.m31, m32 + m.m32, m33 + m.m33, m34 + m.m34,
        m41 + m.m41, m42 + m.m42, m43 + m.m43, m44 + m.m44);
}

Mat4 Mat4::operator-(const Mat4& m) const
{
    return Mat4(
        m11 - m.m11, m12 - m.m12, m13 - m.m13, m14 - m.m14,
        m21 - m.m21, m22 - m.m22, m23 - m.m23, m24 - m.m24,
        m31 - m.m31, m32 - m.m32, m33 - m.m33, m34 - m.m34,
        m41 - m.m41, m42 - m.m42, m43 - m.m43, m44 - m.m44);
}

Mat4 Mat4::operator+(float s) const
{
    return Mat4(
        m11 + s, m12 + s, m13 + s, m14 + s,
        m21 + s, m22 + s, m23 + s, m24 + s,
        m31 + s, m32 + s, m33 + s, m34 + s,
        m41 + s, m42 + s, m43 + s, m44 + s);
}

Mat4 Mat4::operator-(float s) const
{
    return Mat4(
        m11 - s, m12 - s, m13 - s, m14 - s,
        m21 - s, m22 - s, m23 - s, m24 - s,
        m31 - s, m32 - s, m33 - s, m34 - s,
        m41 - s, m42 - s, m43 - s, m44 - s);
}

Mat4 Mat4::operator*(float s) const
{
    return Mat4(
        m11 * s, m12 * s, m13 * s, m14 * s,
        m21 * s, m22 * s, m23 * s, m24 * s,
        m31 * s, m32 * s, m33 * s, m34 * s,
        m41 * s, m42 * s, m43 * s, m44 * s);
}

Mat4 Mat4::operator/(float s) const
{
    float ss = 1.0f / s;
    return Mat4(
        m11 * ss, m12 * ss, m13 * ss, m14 * ss,
        m21 * ss, m22 * ss, m23 * ss, m24 * ss,
        m31 * ss, m32 * ss, m33 * ss, m34 * ss,
        m41 * ss, m42 * ss, m43 * ss, m44 * ss);
}

Mat4 Mat4::operator*(const Mat4& m) const
{
#if USE_SSE
    Mat4 ret;
    mmul(&m11, &m.m11, &ret.m11);
	return ret;
#else
    return Mat4(
        m11 * m.m11 + m12 * m.m21 + m13 * m.m31 + m14 * m.m41,
        m11 * m.m12 + m12 * m.m22 + m13 * m.m32 + m14 * m.m42,
        m11 * m.m13 + m12 * m.m23 + m13 * m.m33 + m14 * m.m43,
        m11 * m.m14 + m12 * m.m24 + m13 * m.m34 + m14 * m.m44,

        m21 * m.m11 + m22 * m.m21 + m23 * m.m31 + m24 * m.m41,
        m21 * m.m12 + m22 * m.m22 + m23 * m.m32 + m24 * m.m42,
        m21 * m.m13 + m22 * m.m23 + m23 * m.m33 + m24 * m.m43,
        m21 * m.m14 + m22 * m.m24 + m23 * m.m34 + m24 * m.m44,

        m31 * m.m11 + m32 * m.m21 + m33 * m.m31 + m34 * m.m41,
        m31 * m.m12 + m32 * m.m22 + m33 * m.m32 + m34 * m.m42,
        m31 * m.m13 + m32 * m.m23 + m33 * m.m33 + m34 * m.m43,
        m31 * m.m14 + m32 * m.m24 + m33 * m.m34 + m34 * m.m44,

        m41 * m.m11 + m42 * m.m21 + m43 * m.m31 + m44 * m.m41,
        m41 * m.m12 + m42 * m.m22 + m43 * m.m32 + m44 * m.m42,
        m41 * m.m13 + m42 * m.m23 + m43 * m.m33 + m44 * m.m43,
        m41 * m.m14 + m42 * m.m24 + m43 * m.m34 + m44 * m.m44);
#endif
}

Mat4& Mat4::operator+=(const Mat4& m) {
    return (*this = *this + m);
}

Mat4& Mat4::operator-=(const Mat4& m){
    return (*this = *this - m);
}

Mat4& Mat4::operator+=(float s){
    return (*this = *this + s);
}

Mat4& Mat4::operator-=(float s){
    return (*this = *this - s);
}

Mat4& Mat4::operator*=(float s){
    return (*this = *this * s);
}

Mat4& Mat4::operator/=(float s){
    return (*this = *this / s);
}

Mat4& Mat4::operator*=(const Mat4& m){
    return (*this = *this * m);
}

bool Mat4::operator==(const Mat4& other) const
{
    return
        m11 == other.m11 && m12 == other.m12 && m13 == other.m13 && m14 == other.m14 &&
        m21 == other.m21 && m22 == other.m22 && m23 == other.m23 && m24 == other.m24 &&
        m31 == other.m31 && m32 == other.m32 && m33 == other.m33 && m34 == other.m34 &&
        m41 == other.m41 && m42 == other.m42 && m43 == other.m43 && m44 == other.m44;
}

bool Mat4::operator!=(const Mat4& other) const
{
    return
        m11 != other.m11 || m12 != other.m12 || m13 != other.m13 || m14 != other.m14 ||
        m21 != other.m21 || m22 != other.m22 || m23 != other.m23 || m24 != other.m24 ||
        m31 != other.m31 || m32 != other.m32 || m33 != other.m33 || m34 != other.m34 ||
        m41 != other.m41 || m42 != other.m42 || m43 != other.m43 || m44 != other.m44;
}

void Mat4::Transpose() {
    swap(m12, m21);
    swap(m13, m31);
    swap(m14, m41);
    swap(m23, m32);
    swap(m24, m42);
    swap(m34, m43);
}

Mat4 Mat4::Transposed() const {
    return Mat4(
        m11, m21, m31, m41,
        m12, m22, m32, m42,
        m13, m23, m33, m43,
        m14, m24, m34, m44);
}

Mat4 Mat4::Invert() {
    return (*this = Inverse());
}

Mat4 Mat4::Inverse() const
{
    Mat4 adj;

    adj.m11 = m22 * m33 * m44 - m22 * m34 * m43 - m32 * m23 * m44 + m32 * m24 * m43 + m42 * m23 * m34 - m42 * m24 * m33;
    adj.m21 = -m21 * m33 * m44 + m21 * m34 * m43 + m31 * m23 * m44 - m31 * m24 * m43 - m41 * m23 * m34 + m41 * m24 * m33;
    adj.m31 = m21  *  m32 * m44 - m21 * m34 * m42 - m31 * m22 * m44 + m31 * m24 * m42 + m41 * m22 * m34 - m41 * m24 * m32;
    adj.m41 = -m21 * m32 * m43 + m21 * m33 * m42 + m31 * m22 * m43 - m31 * m23 * m42 - m41 * m22 * m33 + m41 * m23 * m32;
    adj.m12 = -m12 * m33 * m44 + m12 * m34 * m43 + m32 * m13 * m44 - m32 * m14 * m43 - m42 * m13 * m34 + m42 * m14 * m33;
    adj.m22 = m11 * m33 * m44 - m11 * m34 * m43 - m31 * m13 * m44 + m31 * m14 * m43 + m41 * m13 * m34 - m41 * m14 * m33;
    adj.m32 = -m11 * m32 * m44 + m11 * m34 * m42 + m31 * m12 * m44 - m31 * m14 * m42 - m41 * m12 * m34 + m41 * m14 * m32;
    adj.m42 = m11 * m32 * m43 - m11 * m33 * m42 - m31 * m12 * m43 + m31 * m13 * m42 + m41 * m12 * m33 - m41 * m13 * m32;
    adj.m13 = m12 * m23 * m44 - m12 * m24 * m43 - m22 * m13 * m44 + m22 * m14 * m43 + m42 * m13 * m24 - m42 * m14 * m23;
    adj.m23 = -m11 * m23 * m44 + m11 * m24 * m43 + m21 * m13 * m44 - m21 * m14 * m43 - m41 * m13 * m24 + m41 * m14 * m23;
    adj.m33 = m11 * m22 * m44 - m11 * m24 * m42 - m21 * m12 * m44 + m21 * m14 * m42 + m41 * m12 * m24 - m41 * m14 * m22;
    adj.m43 = -m11 * m22 * m43 + m11 * m23 * m42 + m21 * m12 * m43 - m21 * m13 * m42 - m41 * m12 * m23 + m41 * m13 * m22;
    adj.m14 = -m12 * m23 * m34 + m12 * m24 * m33 + m22 * m13 * m34 - m22 * m14 * m33 - m32 * m13 * m24 + m32 * m14 * m23;
    adj.m24 = m11 * m23 * m34 - m11 * m24 * m33 - m21 * m13 * m34 + m21 * m14 * m33 + m31 * m13 * m24 - m31 * m14 * m23;
    adj.m34 = -m11 * m22 * m34 + m11 * m24 * m32 + m21 * m12 * m34 - m21 * m14 * m32 - m31 * m12 * m24 + m31 * m14 * m22;
    adj.m44 = m11 * m22 * m33 - m11 * m23 * m32 - m21 * m12 * m33 + m21 * m13 * m32 + m31 * m12 * m23 - m31 * m13 * m22;

    float det = m11 * adj.m11 + m12 * adj.m21 + m13 * adj.m31 + m14 * adj.m41;
    if(abs(det) < FLT_EPSILON)
        return *this;

    return adj * (1.0f / det);
}

Mat4 Mat4::Scale(float x, float y, float z)
{
    return Mat4(
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, z, 0,
        0, 0, 0, 1);
}

Mat4 Mat4::Translation(float x, float y, float z)
{
    return Mat4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        x, y, z, 1);
}

Mat4 Mat4::Rotation(float x, float y, float z)
{
    float rx = x * DegToRad;
    float ry = y * DegToRad;
    float rz = z * DegToRad;

    float sx = sin(rx);
    float sy = sin(ry);
    float sz = sin(rz);
    float cx = cos(rx);
    float cy = cos(ry);
    float cz = cos(rz);

    return Mat4(
        (cy * cz + sx * sy * sz), (cx * sz), (sx * cy * sz - sy * cz), 0,
        (sx * sy * cz - cy * sz), (cx * cz), (sx * cy * cz + sy * sz), 0,
        (cx * sy), (-sx), (cx * cy), 0,
        0, 0, 0, 1);
}

Mat4 Mat4::XRotation(float angle)
{
    float ha = angle * DegToRad;
    float s = sin(ha);
    float c = cos(ha);
    
    return Mat4(1, 0, 0, 0,
                0, c, s, 0,
                0,-s, c, 0,
                0, 0, 0, 1);
}

Mat4 Mat4::YRotation(float angle)
{
    float ha = angle * DegToRad;
    float s = sin(ha);
    float c = cos(ha);
        
    return Mat4(c, 0,-s, 0,
                0, 1, 0, 0,
                s, 0, c, 0,
                0, 0, 0, 1);
}

Mat4 Mat4::ZRotation(float angle)
{
    float ha = angle * DegToRad;
    float s = sin(ha);
    float c = cos(ha);

    return Mat4(c, s, 0, 0,
               -s, c, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1);
}

Mat4 Mat4::LookRotation(const Vec3& forward, const Vec3& upward)
{
    Vec3 zaxis = forward.Normalized();
    Vec3 xaxis = upward.Cross(zaxis).Normalized();
    Vec3 yaxis = zaxis.Cross(xaxis);
        
    return Mat4(xaxis.x, xaxis.y, xaxis.z, 0,
                yaxis.x, yaxis.y, yaxis.z, 0,
                zaxis.x, zaxis.y, zaxis.z, 0,
                0,       0,       0,       1);
}

Mat4 Mat4::Transform(const Vec3& pos, const Vec3& scale, const Quat& rot)
{
    Mat4 mtx = rot.ToMatrix();
    
    mtx.m11 *= scale.x;
    mtx.m12 *= scale.x;
    mtx.m13 *= scale.x;
    
    mtx.m21 *= scale.y;
    mtx.m22 *= scale.y;
    mtx.m23 *= scale.y;
    
    mtx.m31 *= scale.z;
    mtx.m32 *= scale.z;
    mtx.m33 *= scale.z;

    mtx.m41 = pos.x;
    mtx.m42 = pos.y;
    mtx.m43 = pos.z;

    return mtx;
}

Mat4 Mat4::InverseTransform(const Vec3& pos, const Vec3& scale, const Quat& rotation)
{
    Quat rot = rotation;
    rot.Invert();
    
    Vec3 npos = -pos;
    
    Vec3 scl(1.0f / scale.x,
             1.0f / scale.y,
             1.0f / scale.z);
    
    Mat4 mtx = rot.ToMatrix();
    
    mtx.m11 *= scl.x;
    mtx.m12 *= scl.x;
    mtx.m13 *= scl.x;
    
    mtx.m21 *= scl.y;
    mtx.m22 *= scl.y;
    mtx.m23 *= scl.y;
    
    mtx.m31 *= scl.z;
    mtx.m32 *= scl.z;
    mtx.m33 *= scl.z;
    
    mtx.m41 = (npos.x * mtx.m11) + (npos.y * mtx.m21) + (npos.z * mtx.m31);
    mtx.m42 = (npos.x * mtx.m12) + (npos.y * mtx.m22) + (npos.z * mtx.m32);
    mtx.m43 = (npos.x * mtx.m13) + (npos.y * mtx.m23) + (npos.z * mtx.m33);

    return mtx;
}

Mat4 Mat4::Ortho2D(float left, float right, float bottom, float top, float near, float far)
{
    float w = right - left;
    float h = top - bottom;
    float d = far - near;

    if(abs(w) < Math::FloatTolerance
    || abs(h) < Math::FloatTolerance
    || abs(d) < Math::FloatTolerance)
        return Mat4::identity;

    float sx = 2.0f / w;
    float sy = 2.0f / h;
    float sz = 1.0f / d;

    float tx = -(right + left) / w;
    float ty = -(top + bottom) / h;
    float tz = -near / d;

    return Mat4(sx, 0,  0,  0,
                0,  sy, 0,  0,
                0,  0,  sz, 0,
                tx, ty, tz, 1);
}

Mat4 Mat4::Project3D(float fov, float width, float height, float near, float far) {
    return Mat4::Project3D(fov, width / height, near, far);
}

Mat4 Mat4::Project3D(float fov, float aspect, float near, float far)
{
    float sx = 1.0f / tan(DegToRad * fov / 2.0f);
    float sy = sx * aspect;
    float sz = far / (far - near);
    float tz = -(near * sz);

    return Mat4(sx, 0,  0,  0,
                0,  sy, 0,  0,
                0,  0,  sz, 1,
                0,  0,  tz, 0);
}

std::ostream& operator<<(std::ostream& os, const Mat4& m) {
    os << "[" << m.m11 << ", " << m.m12 << ", " << m.m13 << ", " << m.m14 << "]" << std::endl;
    os << "[" << m.m21 << ", " << m.m22 << ", " << m.m23 << ", " << m.m24 << "]" << std::endl;
    os << "[" << m.m31 << ", " << m.m32 << ", " << m.m33 << ", " << m.m34 << "]" << std::endl;
    os << "[" << m.m41 << ", " << m.m42 << ", " << m.m43 << ", " << m.m44 << "]";
    return os;
}

////////////////////////////////
//    Quat
////////////////////////////////

const Quat Quat::zero(0, 0, 0, 0);
const Quat Quat::identity(0, 0, 0, 1);

Quat::Quat(float xAngle, float yAngle, float zAngle)
{
    float zDeg = zAngle * DegToRad;
    float xDeg = xAngle * DegToRad;
    float yDeg = yAngle * DegToRad;
    
    float hsinz = sin(zDeg * 0.5f);
    float hsinx = sin(xDeg * 0.5f);
    float hsiny = sin(yDeg * 0.5f);
    
    float hcosz = cos(zDeg * 0.5f);
    float hcosx = cos(xDeg * 0.5f);
    float hcosy = cos(yDeg * 0.5f);

    w   = hcosy * hcosx * hcosz + hsiny * hsinx * hsinz;
    v.x = hcosy * hsinx * hcosz + hsiny * hcosx * hsinz;
    v.y = hsiny * hcosx * hcosz - hcosy * hsinx * hsinz;
    v.z = hcosy * hcosx * hsinz - hsiny * hsinx * hcosz;
}

Quat::Quat(float x, float y, float z, float w)
    : v(x, y, z), w(w)
{
}

Quat::Quat(const Vec3& v, float w)
    : v(v), w(w)
{
}

Quat::Quat(const Vec3& angles)
        : Quat(angles.x, angles.y, angles.z)
{
}

Quat Quat::operator+(const Quat& q) const
{
    return Quat(v + q.v, w + q.w);
}

Quat Quat::operator-(const Quat& q) const
{
    return Quat(v - q.v, w - q.w);
}

Quat Quat::operator*(const Quat& q) const
{
    return Quat(v.z * q.v.y - v.y * q.v.z + v.x * q.w + w * q.v.x,
                v.x * q.v.z - v.z * q.v.x + v.y * q.w + w * q.v.y,
                v.y * q.v.x - v.x * q.v.y + v.z * q.w + w * q.v.z,
                w   * q.w   - v   * q.v);
}

Quat Quat::operator/(const Quat& q) const
{
    return *this * q.Inverse();
}

Quat Quat::operator*(float s) const
{
    return Quat(v * s, w * s);
}

Quat Quat::operator/(float s) const
{
    return Quat(v / s, w / s);
}

Quat Quat::operator-() const
{
    return Quat(-v, -w);
}

const Quat& Quat::operator+=(const Quat& q)
{
    v += q.v;
    w += q.w;
    return *this;
}

const Quat& Quat::operator-=(const Quat& q)
{
    v -= q.v;
    w -= q.w;
    return *this;
}

const Quat& Quat::operator*=(const Quat& q)
{
    Quat c = *this;
    v.x = c.v.z * q.v.y - c.v.y * q.v.z + c.v.x * q.w + c.w * q.v.x;
    v.y = c.v.x * q.v.z - c.v.z * q.v.x + c.v.y * q.w + c.w * q.v.y;
    v.z = c.v.y * q.v.x - c.v.x * q.v.y + c.v.z * q.w + c.w * q.v.z;
    w   = c.w   * q.w   - c.v   * q.v;
	return *this;
}

const Quat& Quat::operator*=(float s)
{
    v *= s;
    w *= s;
    return *this;
}

const Quat& Quat::operator/=(float s)
{
    v /= s;
    w /= s;
    return *this;
}

float Quat::Dot(const Quat & q) const
{
    return v * q.v + w * q.w;
}

Mat3 Quat::ToMatrix() const
{
    float xx = v.x * v.x;
    float xy = v.x * v.y;
    float xz = v.x * v.z;
    float xw = v.x * w;
    float yy = v.y * v.y;
    float yz = v.y * v.z;
    float yw = v.y * w;
    float zz = v.z * v.z;
    float zw = v.z * w;

    return Mat3(1 - 2 * (zz + yy), 2 * (xy + zw), 2 * (xz - yw),
                2 * (xy - zw), 1 - 2 * (zz + xx), 2 * (yz + xw),
                2 * (yw + xz), 2 * (yz - xw), 1 - 2 * (yy + xx));
}

float Quat::Length() const
{
    return sqrt(v * v + w * w);
}

float Quat::LengthSq() const
{
    return (v * v + w * w);
}

void Quat::Normalize()
{
    *this /= Length();
}

Quat Quat::Normalized() const
{
    return *this / Length();
}

void Quat::Invert()
{
    v = -v;
    *this *= (1.0f / (v * v + w * w));
}

Quat Quat::Inverse() const
{
    float s = 1.0f / (v * v + w * w);
    return Quat(-v * s, w * s);
}

Quat Quat::AngleAxis(float angle, const Vec3& axis) {
    float ha = angle * 0.5f * DegToRad;
    return Quat(axis * sin(ha), cos(ha));
}

Quat Quat::FromTo(const Vec3& from, const Vec3& to)
{
    Vec3 v0 = from.Normalized();
    Vec3 v1 = to.Normalized();
    float dot = v0.Dot(v1);

    if(dot >= 1.0f - FLT_EPSILON)
    {
        return Quat::identity;
    }
    else if(dot <= -1.0f + FLT_EPSILON)
    {
        Vec3 axis = Vec3(1, 0, 0).Cross(v0);

        if(axis.LengthSq() < FLT_EPSILON * 2)
            axis = Vec3(0, 1, 0).Cross(v0);

        axis.Normalize();
        return Quat(axis, 0);
    }

    float s = sqrt((1.0f + dot) * 2.0f);
    float invs = 1.0f / s;
    Vec3 c = v0.Cross(v1) * invs;
    return Quat(c, s * 0.5f).Normalized();
}

Quat Quat::LookRotation(const Vec3& forward, const Vec3& upward) {
    return Mat3::LookRotation(forward, upward).GetRotation();
}

Quat Quat::Lerp(const Quat& a, const Quat& b, float t)
{
    return (a * (1 - t) + b * t).Normalized();
}

Quat Quat::Slerp(const Quat& a, const Quat& b, float t)
{
    float dot = a.Dot(b);

    Quat bb;
    if(dot < 0)
    {
        dot = -dot;
        bb = -b;
    }
    else
    {
        bb = b;
    }

    if(dot < 0.95f)
    {
        float angle = acos(dot);
        float sina = sin(angle);
        return (a * sin(angle * (1 - t)) + bb * sin(angle * t)) / sina;
    }
    else
    {
        return Lerp(a, bb, t);
    }
}

void Quat::ToAngleAxis(float& angle, Vec3& axis) const
{
    float a = acos(w);
    axis = v * (1.0f / sin(a));
    angle = a * 2.0f * RadToDeg;
}

Vec3 Quat::ToEulerAngles() const
{
    float test = v.y * v.z - w * v.x;
	if(test > 0.499f)
    { // singularity at north pole
        // ??
		// heading = 2 * atan2(x, w);
		// attitude = Pi / 2;
		// bank = 0;
		//return Vec3::zero;
	}
    else if(test < -0.499f)
    { // singularity at south pole
        // ??
		// heading = -2 * atan2(v.x, w);
		// attitude = - Pi / 2;
		// bank = 0;
		//return Vec3::zero;
	}

    float xx = v.x * v.x;
    float yy = v.y * v.y;
    float zz = v.z * v.z;
    float ww = w * w;
    
    Vec3 ret;
    ret.x = RadToDeg * asin(std::min(std::max(-2.0f * test, -1.0f), 1.0f));
    ret.y = RadToDeg * atan2(2 * (v.x * v.z + w * v.y), ww - xx - yy + zz);
    ret.z = RadToDeg * atan2(2 * (v.x * v.y + w * v.z), ww - xx + yy - zz);
    return ret;
}

std::ostream& operator<<(std::ostream& os, const Quat& q) {
    os << q.ToEulerAngles();
    return os;
}

////////////////////////////////
//    Ray
////////////////////////////////

bool Ray::Cast(const Plane& p) const
{
    float dot = p.Normal().Dot(direction);
    
    if(dot > -FLT_EPSILON && dot < FLT_EPSILON)
        return false;

    return true;
}

bool Ray::Cast(const Plane& p, Vec3& hitpoint) const
{
    Plane pn = p;
    pn.Normalize();

    Vec3 normal(pn.a, pn.b, pn.c);

    float dot = normal.Dot(direction);
    
    if(dot > -FLT_EPSILON && dot < FLT_EPSILON)
        return false;

    float t = (normal.Dot(origin) + p.d) / -normal.Dot(direction);

    hitpoint = origin + direction * t;

    return true;
}

bool Ray::Cast(const Sphere& s) const
{
    Vec3 sp(s.center.x, s.center.y, s.center.z);
    
    float t = (sp - origin).Dot(direction) / direction.Dot(direction);

    if(t < 0.0f)
        return false;
    
    Vec3 worldPt = origin + (direction * t);
    
    return sp.DistanceSq( worldPt ) <= (s.radius * s.radius);
}

bool Ray::Cast(const Sphere& s, Vec3& hitpoint) const
{
    Vec3 loc_orig = origin - s.center;

    float a = direction.Dot(direction);
    float b = 2.0f * direction.Dot(loc_orig);
    float c = loc_orig.Dot(loc_orig) - (s.radius * s.radius);
    
    float discr = b * b - 4.0f * a * c;

    if(discr < -FLT_EPSILON) // < 0
    {
        return false;
    }
    else if(discr < FLT_EPSILON) // == 0
    {
        float t = -b / (2 * a);
        hitpoint = origin + direction * t;
    }
    else // > 0
    {
        discr = sqrt(discr);

        float q = (b > 0)? (0.5f * (-b + discr)) : (0.5f * (-b - discr));
        float t0 = q / a;
        float t1 = c / q;

        if(t0 > t1)
        {
            float tmp = t0;
            t0 = t1;
            t1 = tmp;
        }

        hitpoint = origin + direction * t0;
    }

    return true;
}

bool Ray::Cast(const Triangle& tri) const
{
    Vec3 side_1 = tri.b - tri.a;
    Vec3 side_2 = tri.c - tri.a;
    
    Vec3 pvec = direction.Cross(side_2);
    float det = side_1.Dot( pvec );

    if(det < 0.000001f)
        return false;

    Vec3 tvec = origin - tri.a;
    float u = tvec.Dot(pvec);

    if(u < 0.0f || u > det)
        return false;

    Vec3 qvec = tvec.Cross(side_1);
    float v = direction.Dot(qvec);

    if(v < 0 || u + v > det)
        return false;
    
    return true;
}

bool Ray::Cast(const Triangle& tri, Vec3& hitpoint) const
{
    Vec3 side_1 = tri.b - tri.a;
    Vec3 side_2 = tri.c - tri.a;
    
    Vec3 pvec = direction.Cross(side_2);
    float det = side_1.Dot( pvec );

    if(det < 0.000001f)
        return false;

    Vec3 tvec = origin - tri.a;
    float u = tvec.Dot(pvec);

    if(u < 0.0f || u > det)
        return false;

    Vec3 qvec = tvec.Cross(side_1);
    float v = direction.Dot(qvec);

    if(v < 0 || u + v > det)
        return false;
    
    float t = side_2.Dot(qvec) / det;

    //det = 1.0f / det;
    //t *= det;
    //u *= det;
    //v *= det;
    
    /* calculate the point of intersection between the ray and the triangle */
    //Vec3 intersection = tri.a + (side_1 * u) + (side_2 * v);

    /* or use t to find the intersection */
    hitpoint = direction * t + origin;

    return true;
}

bool Ray::CastNoCull(const Triangle& tri) const
{
    Vec3 side_1 = tri.b - tri.a;
    Vec3 side_2 = tri.c - tri.a;
    
    Vec3 pvec = direction.Cross(side_2);
    float det = side_1.Dot( pvec );

    if(fabs(det) < 0.000001f)
        return false;

    det = 1.0f / det;

    Vec3 tvec = origin - tri.a;
    float u = tvec.Dot(pvec) * det;

    if(u < 0.0f || u > 1.0f)
        return false;

    Vec3 qvec = tvec.Cross(side_1);
    float v = direction.Dot(qvec) * det;

    if(v < 0.0f || u + v > 1.0f)
        return false;
    
    return true;
}

bool Ray::CastNoCull(const Triangle& tri, Vec3& hitpoint) const
{
    Vec3 side_1 = tri.b - tri.a;
    Vec3 side_2 = tri.c - tri.a;
    
    Vec3 pvec = direction.Cross(side_2);
    float det = side_1.Dot( pvec );

    if(fabs(det) < 0.000001f)
        return false;

    det = 1.0f / det;

    Vec3 tvec = origin - tri.a;
    float u = tvec.Dot(pvec) * det;

    if(u < 0.0f || u > 1.0f)
        return false;

    Vec3 qvec = tvec.Cross(side_1);
    float v = direction.Dot(qvec) * det;

    if(v < 0.0f || u + v > 1.0f)
        return false;
    
    float t = side_2.Dot(qvec) * det;

    /* calculate the point of intersection between the ray and the triangle */
    //Vec3 intersection = tri.a + (side_1 * u) + (side_2 * v);

    /* or use t to find the intersection */
    hitpoint = direction * t + origin;

    return true;
}

////////////////////////////////
//    Plane
////////////////////////////////

void Plane::Normalize()
{
    float length = sqrt(a * a + b * b + c * c);
        
    if(length > 0.0f)
    {
        length = 1.0f / length;
        a *= length;
        b *= length;
        c *= length;
        d *= length;
    }
}

Vec3 Plane::Normal() const
{
    return Vec3(a, b, c).Normalized();
}

bool Plane::InFront(const Vec3& point) const
{
    return (point.x * a + point.y * b + point.z * c + d) > 0.0f;
}

bool Plane::InBack(const Vec3& point) const
{
    return (point.x * a + point.y * b + point.z * c + d) < 0.0f;
}

bool Plane::InFront(const Sphere& sphere) const
{
    return (sphere.center.x * a +
            sphere.center.y * b +
            sphere.center.z * c + d) > sphere.radius;
}

bool Plane::InBack(const Sphere& sphere) const
{
    return (sphere.center.x * a +
            sphere.center.y * b +
            sphere.center.z * c + d) < -sphere.radius;
}

Plane::operator Vec4() const
{
    return Vec4(a, b, c, d);
}

////////////////////////////////
//    Triangle
////////////////////////////////



////////////////////////////////
//    Sphere
////////////////////////////////

Sphere Sphere::operator+(const Sphere& other)
{
    Vec3 sep = other.center - center;
    float dist = sep.Length();

    if(dist < FLT_EPSILON)
        return (radius > other.radius) ? *this : other;

    if(this->radius > other.radius + dist)
        return *this;

    if(other.radius > this->radius + dist)
        return other;

    sep /= dist;

    Sphere ret;

    Vec3 ext1 = other.center + sep * other.radius;
    Vec3 ext2 = this->center + -sep * this->radius;
        
    ret.center = ext1 + (ext2 - ext1) * 0.5f;
    ret.radius = 0.5f * (radius + dist + other.radius);

    return ret;
}

Sphere& Sphere::operator+=(const Sphere& other)
{
    Vec3 sep = other.center - center;
    float dist = sep.Length();

    if(dist < FLT_EPSILON)
        return (radius > other.radius) ? *this : *this = other;

    if(this->radius > other.radius + dist)
        return *this;

    if(other.radius > this->radius + dist)
        return *this = other;

    sep /= dist;

    Vec3 ext1 = other.center + sep * other.radius;
    Vec3 ext2 = this->center + -sep * this->radius;
        
    center = ext1 + (ext2 - ext1) * 0.5f;
    radius = 0.5f * (radius + dist + other.radius);

    return *this;
}

////////////////////////////////
//    Box
////////////////////////////////

void Box::GetVerts(Vec3 (&verts)[8]) const
{
    // bottom square
    verts[0] = Vec3(vmin.x, vmin.y, vmin.z);
    verts[1] = Vec3(vmin.x, vmin.y, vmax.z);
    verts[2] = Vec3(vmax.x, vmin.y, vmax.z);
    verts[3] = Vec3(vmax.x, vmin.y, vmin.z);
        
    // top square
    verts[4] = Vec3(vmin.x, vmax.y, vmin.z);
    verts[5] = Vec3(vmin.x, vmax.y, vmax.z);
    verts[6] = Vec3(vmax.x, vmax.y, vmax.z);
    verts[7] = Vec3(vmax.x, vmax.y, vmin.z);
}

Box Box::operator+(const Box& other)
{
    Box ret;
        
    ret.vmin.x = min(vmin.x, other.vmin.x);
    ret.vmin.y = min(vmin.y, other.vmin.y);
    ret.vmin.z = min(vmin.z, other.vmin.z);

    ret.vmax.x = min(vmax.x, other.vmax.x);
    ret.vmax.y = min(vmax.y, other.vmax.y);
    ret.vmax.z = min(vmax.z, other.vmax.z);
        
    return ret;
}

Box& Box::operator+=(const Box& other)
{
    vmin.x = min(vmin.x, other.vmin.x);
    vmin.y = min(vmin.y, other.vmin.y);
    vmin.z = min(vmin.z, other.vmin.z);

    vmax.x = max(vmax.x, other.vmax.x);
    vmax.y = max(vmax.y, other.vmax.y);
    vmax.z = max(vmax.z, other.vmax.z);
        
    return *this;
}

////////////////////////////////
//    Rect
////////////////////////////////




//////////////////////////////////////
//    ColorBGRA
//////////////////////////////////////

ColorBGRA::ColorBGRA(uint32_t c) {
    *(uint32_t*)this = c;
}

ColorBGRA::operator uint32_t() const {
    return *(uint32_t*)this;
}

//////////////////////////////////////
//    Color32
//////////////////////////////////////

Color32::Color32(uint32_t c)
{
    ColorBGRA bgra = c;
    b = bgra.b;
    g = bgra.g;
    r = bgra.r;
    a = bgra.a;
}

Color32::Color32(ColorBGRA c)
{
    b = c.b;
    g = c.g;
    r = c.r;
    a = c.a;
}

Color32::operator uint32_t() const {
    return ColorBGRA(b, g, r, a);
}

Color32::operator ColorBGRA() const {
    return ColorBGRA(b, g, r, a);
}

//////////////////////////////////////
//    Color
//////////////////////////////////////

const Color Color::red(1.0f, 0.0f, 0.0f, 1.0f);
const Color Color::green(0.0f, 1.0f, 0.0f, 1.0f);
const Color Color::blue(0.0f, 0.0f, 1.0f, 1.0f);
const Color Color::yellow(1.0f, 1.0f, 0.0f, 1.0f);
const Color Color::orange(1.0f, 0.5f, 0.0f, 1.0f);
const Color Color::purple(0.5f, 0.0f, 0.5f, 1.0f);
const Color Color::magenta(1.0f, 0.0f, 1.0f, 1.0f);
const Color Color::cyan(0.0f, 1.0f, 1.0f, 1.0f);
const Color Color::black(0.0f, 0.0f, 0.0f, 1.0f);
const Color Color::white(1.0f, 1.0f, 1.0f, 1.0f);
const Color Color::gray(0.5f, 0.5f, 0.5f, 1.0f);
const Color Color::clear(0.0f, 0.0f, 0.0f, 0.0f);

Color::Color(Color32 c)
{
#if USE_SSE
    vcvt_rgba((uint32_t&)c, &r);
#else
    float invChanMax = 1.0f / 255.0f;
    r = invChanMax * (float)c.r;
    g = invChanMax * (float)c.g;
    b = invChanMax * (float)c.b;
    a = invChanMax * (float)c.a;
#endif
}

Color::Color(uint32_t c)
{
#if USE_SSE
    vcvt_bgra_to_rgba(c, &r);
#else
    float invChanMax = 1.0f / 255.0f;
    b = invChanMax * (float)(c & 0xFF);
    g = invChanMax * (float)((c & 0xFF00) >> 8);
    r = invChanMax * (float)((c & 0xFF0000) >> 16);
    a = invChanMax * (float)((c & 0xFF000000) >> 24);
#endif
}

Color& Color::operator=(const Vec4& v)
{
    r = v.x;
    g = v.y;
    b = v.z;
    a = v.w;
    return *this;
}

Color Color::Blend(const Color& dst) const
{
    Color ret;
    float invA = 1.0f - a;
    ret.r = r * a + dst.r * invA;
    ret.g = g * a + dst.g * invA;
    ret.b = b * a + dst.b * invA;
    ret.a = a + dst.a * invA;
    return ret;
}

Color Color::Lerp(const Color& a, const Color& b, float t)
{
    return a + (b - a) * t;
}

Color Color::Clamp(const Color& c, float lower, float upper)
{
#if USE_SSE
    Color ret;
    vclamp(&c.r, lower, upper, &ret.r);
    return ret;
#else
    Color ret;
    ret.r = Math::Clamp(c.r, 0.0f, 1.0f);
    ret.g = Math::Clamp(c.g, 0.0f, 1.0f);
    ret.b = Math::Clamp(c.b, 0.0f, 1.0f);
    ret.a = Math::Clamp(c.a, 0.0f, 1.0f);
    return ret;
#endif
}

Color::operator uint32_t()
{
#if USE_SSE
    return vcvt_rgba_to_bgra(&r);
#else
    uint32_t bb = ((uint32_t)(b * 255.0f) & 0xFF);
    uint32_t gg = ((uint32_t)(g * 255.0f) & 0xFF) << 8;
    uint32_t rr = ((uint32_t)(r * 255.0f) & 0xFF) << 16;
    uint32_t aa = ((uint32_t)(a * 255.0f) & 0xFF) << 24;
    return bb | gg | rr | aa;
#endif
}

Color::operator Color32()
{
#if USE_SSE
    uint32_t ret = vcvt_rgba(&r);
    return (Color32&)ret;
#else
    uint32_t bb = ((uint32_t)(b * 255.0f) & 0xFF);
    uint32_t gg = ((uint32_t)(g * 255.0f) & 0xFF) << 8;
    uint32_t rr = ((uint32_t)(r * 255.0f) & 0xFF) << 16;
    uint32_t aa = ((uint32_t)(a * 255.0f) & 0xFF) << 24;
    return bb | gg | rr | aa;
#endif
}

Color::operator Vec4&() {
    return *(Vec4*)this;
}

Color::operator float*() {
    return (float*)this;
}

Color Color::operator+(const Color& c) const
{
#if USE_SSE
    Color ret;
    vadd(&r, &c.r, &ret.r);
    return ret;
#else
    return Color(r + c.r, g + c.g, b + c.b, a + c.a);
#endif
}

Color Color::operator-(const Color& c) const
{
#if USE_SSE
    Color ret;
    vsub(&r, &c.r, &ret.r);
    return ret;
#else
    return Color(r - c.r, g - c.g, b - c.b, a - c.a);
#endif
}

Color Color::operator*(float s) const
{
#if USE_SSE
    Color ret;
    vmul(&r, s, &ret.r);
    return ret;
#else
    return Color(r * s, g * s, b * s, a * s);
#endif
}

Color Color::operator*(const Color& c) const
{
#if USE_SSE
    Color ret;
    vmul(&r, &c.r, &ret.r);
    return ret;
#else
    return Color(r * c.r, g * c.g, b * c.b, a * c.a);
#endif
}

Color& Color::operator*=(float s)
{
#if USE_SSE
    vmul(&r, s, &r);
#else
    r *= s;
    g *= s;
    b *= s;
    a *= s;
#endif
    return *this;
}

Color& Color::operator*=(const Color& c)
{
#if USE_SSE
    vmul(&r, &c.r, &r);
#else
    r *= c.r;
    g *= c.g;
    b *= c.b;
    a *= c.a;
#endif
    return *this;
}

Color& Color::operator+=(const Color& c)
{
#if USE_SSE
    vadd(&r, &c.r, &r);
#else
    r += c.r;
    g += c.g;
    b += c.b;
    a += c.a;
#endif
    return *this;
}

////////////////////////////////
//    Misc
////////////////////////////////

float Random::value() {
    constexpr static float invRandMax = 1.0f / (float)RAND_MAX;
    return (float)rand() * invRandMax;
}

float Random::signedValue() {
    constexpr static float invRandMax = 1.0f / (float)RAND_MAX;
    return (float)rand() * invRandMax * 2.0f - 1.0f;
}

Vec2 Random::vector2()
{
    float a = value() * 2.0f * Pi;
    
    float r = value() + value();
    r = r > 1 ? 2 - r : r;

    return Vec2(r * cos(a), r * sin(a));
}

Vec3 Random::vector3()
{
    float a = value() * 2.0f * Pi;

    float r = value() + value();
    r = r > 1 ? 2 - r : r;
    
    float sa = sin(a);
    float ca = cos(a);

    return Vec3(r * ca * sa, r * sa * sa, r * ca);
}

float Random::range(float min, float max)
{
    constexpr static float invRandMax = 1.0f / (float)RAND_MAX;
    return min + (float)rand() * invRandMax * (max - min);
}
