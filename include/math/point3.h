
/***************************************************************
* 
***************************************************************/
#pragma once

#include "utility/basic_types.h"
#include <memory.h>
#include <math.h>
/*************************************************************/
#pragma pack(push, 1)

//##############################################################################
//------------------------------------------------------
//------------------------------ 3F点结构
struct _stPoint3F {
public:
    float x;
    float y;
    float z;

public:
    _stPoint3F& getPoint3F() { return *this; }

public:
    //由于有联合使用所以不能有构造函数
    _stPoint3F() { zero(); }

    explicit _stPoint3F(float xyz) { set(xyz, xyz, xyz); }

    _stPoint3F(float _x, float _y, float _z) { set(_x, _y, _z); }

    _stPoint3F(const _stPoint3F& _copy) { set(_copy.x, _copy.y, _copy.z); }

public:
    float len() { return sqrtf(x*x+y*y+z*z); }

    void zero() { x = y = z = 0.0f; }

    void set(float xyz) { x = y = z = xyz; }

    void set(const _stPoint3F& copy) { set(copy.x, copy.y, copy.z); }

    void set(float _x, float _y, float _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }

    void setMin(const _stPoint3F& _test)
    {
        x = (_test.x<x) ? _test.x : x;
        y = (_test.y<y) ? _test.y : y;
        z = (_test.z<z) ? _test.z : z;
    }

    void setMax(const _stPoint3F& _test)
    {
        x = (_test.x>x) ? _test.x : x;
        y = (_test.y>y) ? _test.y : y;
        z = (_test.z>z) ? _test.z : z;
    }

public:
    void normalize()
    {
        float squared = x*x+y*y+z*z;
        if (squared!=0.0f)
        {
            float factor = 1.0f/sqrtf(squared);
            x *= factor;
            y *= factor;
            z *= factor;
        }
        else
        {
            zero();
        }
    }

public:
    //比较运算符
    bool operator==(const _stPoint3F& _test) const { return (x==_test.x) && (y==_test.y) && (z==_test.z); }

    bool operator!=(const _stPoint3F& _test) const { return operator==(_test)==false; }

public:
    //算术点
    _stPoint3F operator+(const _stPoint3F& _add) const
    {
        _stPoint3F p;
        p.set(x+_add.x, y+_add.y, z+_add.z);
        return p;
    }

    _stPoint3F operator-(const _stPoint3F& _reduce) const
    {
        _stPoint3F p;
        p.set(x-_reduce.x, y-_reduce.y, z-_reduce.z);
        return p;
    }

    _stPoint3F& operator+=(const _stPoint3F& _add)
    {
        x += _add.x;
        y += _add.y;
        z += _add.z;
        return *this;
    }

    _stPoint3F& operator-=(const _stPoint3F& _reduce)
    {
        x -= _reduce.x;
        y -= _reduce.y;
        z -= _reduce.z;
        return *this;
    }

public:
    //算术标量
    _stPoint3F operator*(float _mul) const
    {
        _stPoint3F p;
        p.set(x*_mul, y*_mul, z*_mul);
        return p;
    }

    _stPoint3F operator/(float _div) const
    {
        float    inv = 1.0f/_div;
        _stPoint3F s;
        s.set(x*inv, y*inv, z*inv);
        return s;
    }

    _stPoint3F& operator*=(float _mul)
    {
        x *= _mul;
        y *= _mul;
        z *= _mul;
        return *this;
    }

    _stPoint3F& operator/=(float _div)
    {
        float inv = 1.0f/_div;
        x *= inv;
        y *= inv;
        z *= inv;
        return *this;
    }

    _stPoint3F operator*(const _stPoint3F& _vec) const
    {
        _stPoint3F p;
        p.set(x*_vec.x, y*_vec.y, z*_vec.z);
        return p;
    }

    _stPoint3F& operator*=(const _stPoint3F& _vec)
    {
        x *= _vec.x;
        y *= _vec.y;
        z *= _vec.z;
        return *this;
    }

    _stPoint3F operator/(const _stPoint3F& _vec) const
    {
        _stPoint3F p;
        p.set(x/_vec.x, y/_vec.y, z/_vec.z);
        return p;
    }

    _stPoint3F& operator/=(const _stPoint3F& _vec)
    {
        x /= _vec.x;
        y /= _vec.y;
        z /= _vec.z;
        return *this;
    }

public:
    //一元运算符
    _stPoint3F operator-() const
    {
        _stPoint3F p;
        p.set(-x, -y, -z);
        return p;
    }
};
//##############################################################################
#pragma pack(pop)



