
#pragma once

#include "utility/basic_types.h"
#include <memory.h>
#include <math.h>
/*************************************************************/
#pragma pack(push, 1)
//##############################################################################
struct _stPoint2F;
struct _stPoint2I16;

struct _stPoint2I {
public:
    int32_t x;
    int32_t y;

public:
    _stPoint2I& getPoint2I() { return *this; }

public:
    //由于有联合使用所以不能有构造函数
    _stPoint2I() { zero(); }

    explicit _stPoint2I(int32_t xy) { set(xy, xy); }

    _stPoint2I(int32_t _x, int32_t _y) { set(_x, _y); }

    _stPoint2I(const _stPoint2I& _copy) { set(_copy.x, _copy.y); }

    inline _stPoint2I(const _stPoint2F& _copy);

public:
    float len() const { return sqrtf(float(x*x+y*y)); }

    void zero() { x = y = 0; }

    bool isZero() { return (x==0 && y==0); }

    void set(int32_t xy) { x = y = xy; }

    void set(const _stPoint2I& copy) { set(copy.x, copy.y); }

    void set(int32_t _x, int32_t _y)
    {
        x = _x;
        y = _y;
    }

    void setMin(const _stPoint2I& _test)
    {
        x = (_test.x<x) ? _test.x : x;
        y = (_test.y<y) ? _test.y : y;
    }

    void setMax(const _stPoint2I& _test)
    {
        x = (_test.x>x) ? _test.x : x;
        y = (_test.y>y) ? _test.y : y;
    }

public:
    void normalize(int nUint)
    {
        float squared = float(x*x+y*y);
        if (squared!=0.0f)
        {
            float factor = float(nUint);
            factor /= sqrtf(squared);
            x              = int32_t(x*factor);
            y              = int32_t(y*factor);
        }
        else
        {
            zero();
        }
    }

    //--- 填写为整数
    void fillInteger(int32_t _x, int32_t _y)
    {
        if (!_x || !_y)
            return;

        int32_t x_ = int32_t(this->x)/_x+(((int32_t(this->x)%_x)>0) ? 1 : 0);
        int32_t y_ = int32_t(this->y)/_y+(((int32_t(this->y)%_y)>0) ? 1 : 0);

        this->x = x_*_x;
        this->y = y_*_y;
    }

public:
    inline _stPoint2I& operator=(const _stPoint2F& _point);

public:
    //比较运算符
    inline bool operator==(const _stPoint2I& _test) const { return (x==_test.x) && (y==_test.y); }

    inline bool operator!=(const _stPoint2I& _test) const { return operator==(_test)==false; }

public:
    //算术点
    inline _stPoint2I operator+(const _stPoint2I& _add) const
    {
        _stPoint2I p;
        p.set(x+_add.x, y+_add.y);
        return p;
    }

    inline _stPoint2I operator-(const _stPoint2I& _reduce) const
    {
        _stPoint2I p;
        p.set(x-_reduce.x, y-_reduce.y);
        return p;
    }

    inline _stPoint2I& operator+=(const _stPoint2I& _add)
    {
        x += _add.x;
        y += _add.y;
        return *this;
    }

    inline _stPoint2I& operator-=(const _stPoint2I& _reduce)
    {
        x -= _reduce.x;
        y -= _reduce.y;
        return *this;
    }

public:
    //算术标量
    inline _stPoint2I operator*(int32_t _mul) const
    {
        _stPoint2I p;
        p.set(x*_mul, y*_mul);
        return p;
    }

    inline _stPoint2I& operator*=(int32_t _mul)
    {
        x *= _mul;
        y *= _mul;
        return *this;
    }

    inline _stPoint2I& operator/=(int32_t _div)
    {
        float inv = 1.0f/_div;
        x = int32_t(x*inv);
        y = int32_t(y*inv);
        return *this;
    }

    inline _stPoint2I operator*(const _stPoint2I& _vec) const
    {
        _stPoint2I p;
        p.set(x*_vec.x, y*_vec.y);
        return p;
    }

    inline _stPoint2I& operator*=(const _stPoint2I& _vec)
    {
        x *= _vec.x;
        y *= _vec.y;
        return *this;
    }

    inline _stPoint2I operator/(const _stPoint2I& _vec) const
    {
        _stPoint2I p;
        p.set(x/_vec.x, y/_vec.y);
        return p;
    }

    inline _stPoint2I& operator/=(const _stPoint2I& _vec)
    {
        x /= _vec.x;
        y /= _vec.y;
        return *this;
    }

public:
    //一元运算符
    inline _stPoint2I operator-() const
    {
        _stPoint2I p;
        p.set(-x, -y);
        return p;
    }
};

//-------------------------------------------------------------
//------------------------------ 2I16点结构
struct _stPoint2I16 {
public:
    int16_t x;
    int16_t y;

public:
    //由于有联合使用所以不能有构造函数
    _stPoint2I16() { zero(); }

public:
    void zero() { x = y = 0; }

    void set(int16_t _x, int16_t _y)
    {
        x = _x;
        y = _y;
    }

public:
    inline _stPoint2I16& operator=(const _stPoint2F& _point);

    inline _stPoint2I16& operator=(const _stPoint2I& _point);
};

//------------------------------------------------------
//------------------------------ 2F点结构
struct _stPoint2F {
public:
    float x;
    float y;

public:
    _stPoint2F& getPoint2F() { return *this; }

public:
    //由于有联合使用所以不能有构造函数
    _stPoint2F() { zero(); }

    explicit _stPoint2F(float xy) { set(xy, xy); }

    _stPoint2F(float _x, float _y) { set(_x, _y); }

    _stPoint2F(const _stPoint2F& _copy) { set(_copy.x, _copy.y); }

    inline _stPoint2F(const _stPoint2I& _copy);

public:
    float len() { return sqrtf(x*x+y*y); }

    void zero() { x = y = 0.0f; }

    void set(float xy) { x = y = xy; }

    void set(const _stPoint2F& copy) { set(copy.x, copy.y); }

    void set(float _x, float _y)
    {
        x = _x;
        y = _y;
    }

    void setMin(const _stPoint2F& _test)
    {
        x = (_test.x<x) ? _test.x : x;
        y = (_test.y<y) ? _test.y : y;
    }

    void setMax(const _stPoint2F& _test)
    {
        x = (_test.x>x) ? _test.x : x;
        y = (_test.y>y) ? _test.y : y;
    }

public:
    void normalize(float fUint = 1.0f)
    {
        float squared = x*x+y*y;
        if (squared!=0.0f)
        {
            float factor = fUint/sqrtf(squared);
            x *= factor;
            y *= factor;
        }
        else
        {
            zero();
        }
    }

    //--- 填写为整数
    void fillInteger(int32_t _x, int32_t _y)
    {
        if (!_x || !_y)
            return;

        int32_t x_ = int32_t(this->x)/_x+(((int32_t(this->x)%_x)>0) ? 1 : 0);
        int32_t y_ = int32_t(this->y)/_y+(((int32_t(this->y)%_y)>0) ? 1 : 0);

        this->x = float(x_*_x);
        this->y = float(y_*_y);
    }

public:
    inline _stPoint2F& operator=(const _stPoint2I& _point);

public:
    //比较运算符
    inline bool operator==(const _stPoint2F& _test) const { return (x==_test.x) && (y==_test.y); }

    inline bool operator!=(const _stPoint2F& _test) const { return operator==(_test)==false; }

public:
    //算术点
    inline _stPoint2F operator+(const _stPoint2F& _add) const
    {
        _stPoint2F p;
        p.set(x+_add.x, y+_add.y);
        return p;
    }

    inline _stPoint2F operator-(const _stPoint2F& _reduce) const
    {
        _stPoint2F p;
        p.set(x-_reduce.x, y-_reduce.y);
        return p;
    }

    inline _stPoint2F& operator+=(const _stPoint2F& _add)
    {
        x += _add.x;
        y += _add.y;
        return *this;
    }

    inline _stPoint2F& operator-=(const _stPoint2F& _reduce)
    {
        x -= _reduce.x;
        y -= _reduce.y;
        return *this;
    }

public:
    //算术标量
    inline _stPoint2F operator*(float _mul) const
    {
        _stPoint2F p;
        p.set(x*_mul, y*_mul);
        return p;
    }

    inline _stPoint2F operator/(float _div) const
    {
        float    inv = 1.0f/_div;
        _stPoint2F s;
        s.set(x*inv, y*inv);
        return s;
    }

    inline _stPoint2F& operator*=(float _mul)
    {
        x *= _mul;
        y *= _mul;
        return *this;
    }

    inline _stPoint2F& operator/=(float _div)
    {
        float inv = 1.0f/_div;
        x *= inv;
        y *= inv;
        return *this;
    }

    inline _stPoint2F operator*(const _stPoint2F& _vec) const
    {
        _stPoint2F p;
        p.set(x*_vec.x, y*_vec.y);
        return p;
    }

    inline _stPoint2F& operator*=(const _stPoint2F& _vec)
    {
        x *= _vec.x;
        y *= _vec.y;
        return *this;
    }

    inline _stPoint2F operator/(const _stPoint2F& _vec) const
    {
        _stPoint2F p;
        p.set(x/_vec.x, y/_vec.y);
        return p;
    }

    inline _stPoint2F& operator/=(const _stPoint2F& _vec)
    {
        x /= _vec.x;
        y /= _vec.y;
        return *this;
    }

public:
    //一元运算符
    inline _stPoint2F operator-() const
    {
        _stPoint2F p;
        p.set(-x, -y);
        return p;
    }
};

//-------------------------------------------------------------
//------------------------------ 
inline _stPoint2I::_stPoint2I(const _stPoint2F& _copy) { set(int32_t(_copy.x), int32_t(_copy.y)); }

inline _stPoint2I& _stPoint2I::operator=(const _stPoint2F& _point)
{
    set(int32_t(_point.x), int32_t(_point.y));
    return *this;
}

//-------------------------------------------------------------
//------------------------------ 
inline _stPoint2I16& _stPoint2I16::operator=(const _stPoint2F& _point)
{
    set(int16_t(_point.x), int16_t(_point.y));
    return *this;
}

inline _stPoint2I16& _stPoint2I16::operator=(const _stPoint2I& _point)
{
    set(int16_t(_point.x), int16_t(_point.y));
    return *this;
}

//-------------------------------------------------------------
//------------------------------ 
inline _stPoint2F::_stPoint2F(const _stPoint2I& _copy) { set(float(_copy.x), float(_copy.y)); }

inline _stPoint2F& _stPoint2F::operator=(const _stPoint2I& _point)
{
    set(float(_point.x), float(_point.y));
    return *this;
}

//##############################################################################
#pragma pack(pop)



