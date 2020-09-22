
#pragma once

#include "utility/basic_types.h"
#include <memory.h>
#include <math.h>
/*************************************************************/
#pragma pack(push, 1)

//##############################################################################
//-------------------------------------------------------------
//------------------------------ 区域16位
struct _stRectI16 {
public:
    int16_t pointX;
    int16_t pointY;
    int16_t extentX;
    int16_t extentY;

public:
    _stRectI16& getRectI16() { return *this; }

public:
    void zero()
    {
        pointX  = pointY  = 0;
        extentX = extentY = 0;
    }

    void setPoint(int16_t xy) { pointX = pointY = xy; }

    void setPoint(int16_t _x, int16_t _y)
    {
        pointX = _x;
        pointY = _y;
    }

    void setExtent(int16_t xy) { extentX = extentY = xy; }

    void setExtent(int16_t _x, int16_t _y)
    {
        extentX = _x;
        extentY = _y;
    }
};
//##############################################################################
#pragma pack(pop)



