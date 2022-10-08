#include "math.h"
#include <cmath>

// ------------------------------------------------------------------------------------------ v2f

V2f::V2f() : x(0), y(0)
{
}

V2f::V2f(f32 x, f32 y) : x(x), y(y)
{
}

V2f::~V2f()
{
}

V2f V2f::operator+(V2f &o)
{
    return V2f(x + o.x, y + o.y);
}

V2f V2f::operator-(V2f &o)
{
    return V2f(x - o.x, y - o.y);
}

V2f V2f::operator*(V2f &o)
{
    return V2f(x * o.x, y * o.y);
}

V2f V2f::operator/(V2f &o)
{
    return V2f(x / o.x, y / o.y);
}

V2f V2f::operator+=(V2f &o)
{
    x += o.x;
    y += o.y;
    return *this;
}

V2f V2f::operator-=(V2f &o)
{
    x -= o.x;
    y -= o.y;
    return *this;
}

V2f V2f::operator*=(V2f &o)
{
    x *= o.x;
    y *= o.y;
    return *this;
}

V2f V2f::operator/=(V2f &o)
{
    x /= o.x;
    y /= o.y;
    return *this;
}

V2f V2f::operator*(f32 &o)
{
    return V2f(x * o, y * o);
}

V2f V2f::operator/(f32 &o)
{
    return V2f(x / o, y / o);
}

V2f V2f::operator*=(f32 &o)
{
    x *= o;
    y *= o;
    return *this;
}

V2f V2f::operator/=(f32 &o)
{
    x /= o;
    y /= o;
    return *this;
}

f32 V2f::length()
{
    return sqrtf(x * x + y * y);
}

V2f V2f::normalize()
{
    f32 l = length();
    return V2f(x / l, y / l);
}

f32 V2f::dot(V2f &a, V2f &b)
{
    return a.x * b.x + a.y * b.y;
}

f32 V2f::cross(V2f &a, V2f &b)
{
    return a.x * b.y - a.y * b.x;
}

f32 V2f::distance(V2f &a, V2f &b)
{
    return (b - a).length();
}

f32 V2f::angle(V2f &a, V2f &b)
{
    return acosf(dot(a, b) / (a.length() * b.length()));
}

std::ostream &operator<<(std::ostream &os, const V2f &v)
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
