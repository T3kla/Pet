#include "math.h"
#include <cmath>
#include <iostream>

// ------------------------------------------------------------------------------------------ v2f

v2f::v2f() : x(0), y(0)
{
}

v2f::v2f(f32 x, f32 y) : x(x), y(y)
{
}

v2f::~v2f()
{
}

v2f v2f::operator+(v2f &o)
{
    return v2f(x + o.x, y + o.y);
}

v2f v2f::operator-(v2f &o)
{
    return v2f(x - o.x, y - o.y);
}

v2f v2f::operator*(v2f &o)
{
    return v2f(x * o.x, y * o.y);
}

v2f v2f::operator/(v2f &o)
{
    return v2f(x / o.x, y / o.y);
}

v2f v2f::operator+=(v2f &o)
{
    x += o.x;
    y += o.y;
    return *this;
}

v2f v2f::operator-=(v2f &o)
{
    x -= o.x;
    y -= o.y;
    return *this;
}

v2f v2f::operator*=(v2f &o)
{
    x *= o.x;
    y *= o.y;
    return *this;
}

v2f v2f::operator/=(v2f &o)
{
    x /= o.x;
    y /= o.y;
    return *this;
}

v2f v2f::operator*(f32 &o)
{
    return v2f(x * o, y * o);
}

v2f v2f::operator/(f32 &o)
{
    return v2f(x / o, y / o);
}

v2f v2f::operator*=(f32 &o)
{
    x *= o;
    y *= o;
    return *this;
}

v2f v2f::operator/=(f32 &o)
{
    x /= o;
    y /= o;
    return *this;
}

f32 v2f::length()
{
    return sqrtf(x * x + y * y);
}

v2f v2f::normalize()
{
    f32 l = length();
    return v2f(x / l, y / l);
}

f32 v2f::dot(v2f &a, v2f &b)
{
    return a.x * b.x + a.y * b.y;
}

f32 v2f::cross(v2f &a, v2f &b)
{
    return a.x * b.y - a.y * b.x;
}

f32 v2f::distance(v2f &a, v2f &b)
{
    return (b - a).length();
}

f32 v2f::angle(v2f &a, v2f &b)
{
    return acosf(dot(a, b) / (a.length() * b.length()));
}

std::ostream &operator<<(std::ostream &os, const v2f &v)
{
    os << "(" << v.x << ", " << v.y << ")";
    return os;
}

// ------------------------------------------------------------------------------------------ v3f

// v2f::v2f(f32 x, f32 y) : x(x), y(y)
// {
// }

// v3f::v3f(f32 x, f32 y, f32 z) : x(x), y(y), z(z)
// {
// }

// v3f::v3f(v2f v) : x(v.x), y(v.y), z(0)
// {
// }

// v3f::~v3f()
// {
// }

// v3f v3f::operator+( v3f &o)
// {
//     return v3f(x + o.x, y + o.y, z + o.z);
// }

// v3f v3f::operator-( v3f &o)
// {
//     return v3f(x - o.x, y - o.y, z - o.z);
// }

// v3f v3f::operator*( v3f &o)
// {
//     return v3f(x * o.x, y * o.y, z * o.z);
// }

// v3f v3f::operator/( v3f &o)
// {
//     return v3f(x / o.x, y / o.y, z / o.z);
// }

// v3f v3f::operator+=( v3f &o)
// {
//     x += o.x;
//     y += o.y;
//     z += o.z;
//     return *this;
// }

// v3f v3f::operator-=( v3f &o)
// {
//     x -= o.x;
//     y -= o.y;
//     z -= o.z;
//     return *this;
// }

// v3f v3f::operator*=( v3f &o)
// {
//     x *= o.x;
//     y *= o.y;
//     z *= o.z;
//     return *this;
// }

// v3f v3f::operator/=( v3f &o)
// {
//     x /= o.x;
//     y /= o.y;
//     z /= o.z;
//     return *this;
// }

// v3f v3f::operator+( f32 &o)
// {
//     return v3f(x + o, y + o, z + o);
// }

// v3f v3f::operator-( f32 &o)
// {
//     return v3f(x - o, y - o, z - o);
// }

// v3f v3f::operator*( f32 &o)
// {
//     return v3f(x * o, y * o, z * o);
// }

// v3f v3f::operator/( f32 &o)
// {
//     return v3f(x / o, y / o, z / o);
// }

// v3f v3f::operator+=( f32 &o)
// {
//     x += o;
//     y += o;
//     z += o;
//     return *this;
// }

// v3f v3f::operator-=( f32 &o)
// {
//     x -= o;
//     y -= o;
//     z -= o;
//     return *this;
// }

// v3f v3f::operator*=( f32 &o)
// {
//     x *= o;
//     y *= o;
//     z *= o;
//     return *this;
// }

// v3f v3f::operator/=( f32 &o)
// {
//     x /= o;
//     y /= o;
//     z /= o;
//     return *this;
// }
