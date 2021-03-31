
#include "utility/basic_functions.h"
#include <assert.h>
#include <memory>
#include <stdlib.h>

/*************************************************************/

// 设置数组按位标志
bool SetBitFlag(uint32_t* szFlag, int32_t len, int32_t pos)
{
    if (pos >= (len * 32)) {
        return false;
    }
    int iIdx = pos / 32;
    uint32_t bFlag = 0x01 << (31 - (pos % 32));
    szFlag[iIdx] |= bFlag;

    return true;
}

bool UnsetBitFlag(uint32_t* szFlag, int32_t len, int32_t pos)
{
    if (pos >= (len * 32)) {
        return false;
    }
    int iIdx = pos / 32;
    uint32_t bFlag = 0x01 << (31 - (pos % 32));
    szFlag[iIdx] &= ~bFlag;

    return true;
}

bool IsSetBitFlag(uint32_t* szFlag, int32_t len, int32_t pos)
{
    if (pos >= (len * 32)) {
        return false;
    }
    int iIdx = pos / 32;
    uint32_t bFlag = 0x01 << (31 - (pos % 32));

    return (szFlag[iIdx] & bFlag) > 0;
}

bool ClearBitFlag(uint32_t* szFlag, int32_t len)
{
    memset(szFlag, 0, len * 4);
    return true;
}
