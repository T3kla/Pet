#pragma once

#include <iostream>
#include "var.h"

#define PI 3.14159265358979

struct V2f
{
    f32 x;
    f32 y;

    V2f();
    V2f(f32 x, f32 y);
    ~V2f();

    V2f operator+(V2f &o);
    V2f operator-(V2f &o);
    V2f operator*(V2f &o);
    V2f operator/(V2f &o);

    V2f operator+=(V2f &o);
    V2f operator-=(V2f &o);
    V2f operator*=(V2f &o);
    V2f operator/=(V2f &o);

    V2f operator*(f32 &o);
    V2f operator/(f32 &o);

    V2f operator*=(f32 &o);
    V2f operator/=(f32 &o);

    f32 length();
    V2f normalize();

    static f32 dot(V2f &a, V2f &b);
    static f32 cross(V2f &a, V2f &b);
    static f32 distance(V2f &a, V2f &b);
    static f32 angle(V2f &a, V2f &b);

    friend std::ostream &operator<<(std::ostream &os, const V2f &v);
};

// struct v3f
// {
//     f32 x;
//     f32 y;
//     f32 z;

//     v3f(f32 x = 0, f32 y = 0, f32 z = 0);
//     v3f(v2f v);
//     ~v3f();

//     v3f operator+( v3f &o);
//     v3f operator-( v3f &o);
//     v3f operator*( v3f &o);
//     v3f operator/( v3f &o);

//     v3f operator+=( v3f &o);
//     v3f operator-=( v3f &o);
//     v3f operator*=( v3f &o);
//     v3f operator/=( v3f &o);

//     v3f operator+( f32 &o);
//     v3f operator-( f32 &o);
//     v3f operator*( f32 &o);
//     v3f operator/( f32 &o);

//     v3f operator+=( f32 &o);
//     v3f operator-=( f32 &o);
//     v3f operator*=( f32 &o);
//     v3f operator/=( f32 &o);

//     void normalize();
//     void length();

//     f32 dot( v3f &o);
//     v3f cross( v3f &o);

//     static f32 angle( v3f &a,  v3f &b);
//     static f32 distance( v3f &a,  v3f &b);
//     static f32 dot( v3f &a,  v3f &b);
//     static v3f cross( v3f &a,  v3f &b);
// };
