
/***************************************************************
* 基本函数定义
***************************************************************/
#pragma once

#include "utility/basic_types.h"

// 获取数组长度
template <class T>
int getArrayLen(T& array)
{
    return (sizeof(array) / sizeof(array[0]));
}

// 设置数组按位标志
bool SetBitFlag(uint32_t* szFlag, int32_t len, int32_t pos);
bool UnsetBitFlag(uint32_t* szFlag, int32_t len, int32_t pos);
bool IsSetBitFlag(uint32_t* szFlag, int32_t len, int32_t pos);
bool ClearBitFlag(uint32_t* szFlag, int32_t len);
