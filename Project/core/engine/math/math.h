#pragma once

#include "../define/var.h"

#define PI 3.14159265358979

struct v2f
{
    f32 x;
    f32 y;

    v2f();
    // v2f(f32 x, f32 y);
    // ~v2f();

    // v2f operator+(const v2f &o);
    // v2f operator-(const v2f &o);
    // v2f operator*(const v2f &o);
    // v2f operator/(const v2f &o);

    // v2f operator+=(const v2f &o);
    // v2f operator-=(const v2f &o);
    // v2f operator*=(const v2f &o);
    // v2f operator/=(const v2f &o);

    // v2f operator*(const f32 &o);
    // v2f operator/(const f32 &o);

    // v2f operator*=(const f32 &o);
    // v2f operator/=(const f32 &o);

    // f32 length() const;
    // v2f normalize() const;

    // static f32 dot(const v2f &a, const v2f &b);
    // static f32 cross(const v2f &a, const v2f &b);
    // static f32 distance(const v2f &a, const v2f &b);
    // static f32 angle(const v2f &a, const v2f &b);

    // std::ostream &operator<<(std::ostream &stream);
};

// struct v3f
// {
//     f32 x;
//     f32 y;
//     f32 z;

//     v3f(f32 x = 0, f32 y = 0, f32 z = 0);
//     v3f(v2f v);
//     ~v3f();

//     v3f operator+(const v3f &o);
//     v3f operator-(const v3f &o);
//     v3f operator*(const v3f &o);
//     v3f operator/(const v3f &o);

//     v3f operator+=(const v3f &o);
//     v3f operator-=(const v3f &o);
//     v3f operator*=(const v3f &o);
//     v3f operator/=(const v3f &o);

//     v3f operator+(const f32 &o);
//     v3f operator-(const f32 &o);
//     v3f operator*(const f32 &o);
//     v3f operator/(const f32 &o);

//     v3f operator+=(const f32 &o);
//     v3f operator-=(const f32 &o);
//     v3f operator*=(const f32 &o);
//     v3f operator/=(const f32 &o);

//     void normalize();
//     void length();

//     f32 dot(const v3f &o);
//     v3f cross(const v3f &o);

//     static f32 angle(const v3f &a, const v3f &b);
//     static f32 distance(const v3f &a, const v3f &b);
//     static f32 dot(const v3f &a, const v3f &b);
//     static v3f cross(const v3f &a, const v3f &b);
// };
