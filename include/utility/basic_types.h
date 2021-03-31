
/***************************************************************
* 定义各种基本类型
***************************************************************/
#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NULL
#define NULL 0
#endif //NULL

#ifndef SAFE_DELETE
#define SAFE_DELETE(x)  \
    if (nullptr != x) { \
        delete x;       \
        x = nullptr;    \
    }
#define SAFE_DELETE_ARRAY(x) \
    if (nullptr != x) {      \
        delete[] x;          \
        x = nullptr;         \
    }
#endif

#ifndef BREAK_IF
#define BREAK_IF(x) \
    if (x)          \
        break;
#endif

//#############################################################
//############################## 三角形数据
//#############################################################
#ifndef M_PI
#define M_PI 3.1415926535897932384626433
#endif //M_PI

#ifndef M_2PI
#define M_2PI (M_PI * 2.0)
#endif //M_2PI

#ifndef M_PI_F
#define M_PI_F 3.1415926535897932384626433f
#endif //M_PI_F

#ifndef M_2PI_F
#define M_2PI_F (M_PI_F * 2.0f)
#endif //M_2PI_F

//#############################################################
//############################## 常量
//#############################################################
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

inline unsigned long atoul(char const* str) { return strtoul(str, nullptr, 10); }
inline unsigned long long atoull(char const* str) { return strtoull(str, nullptr, 10); }

enum TimeConstants {
    MINUTE = 60,
    HOUR = MINUTE * 60,
    DAY = HOUR * 24,
    WEEK = DAY * 7,
    MONTH = DAY * 30,
    YEAR = MONTH * 12,
    IN_MILLISECONDS = 1000
};
