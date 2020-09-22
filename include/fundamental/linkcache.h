
#pragma once

#include "fundamental/common.h"

namespace svrlib {
struct CLinkItem {
public:

    //如果在free链表中，指向下一个同长度的free结点，如果不free，则指向同一Key值的下一节点。
    size_t   m_nextOffset;
    //本节点中存放的value长度
    uint32_t m_uiValueSize;
    uint8_t  m_oValue[0];
};

enum CVariableLengthHashTable_Get_Result {
  ITEM_DOES_NOT_EXIST,
  BUFFER_LEGNTH_TOO_SMALL,
  GET_SUCCESS,
};

class CLinkCache {
public:
    CLinkCache(uint8_t* pBaseAddress, size_t uiMemorySize,
            size_t uiValueUnitSize);

    size_t Set(const uint8_t* m_pValue, size_t uiValueLength);

    size_t Set(const uint8_t* m_pValue1, size_t uiValueLength1, const uint8_t* m_pValue2, size_t uiValueLength2);

    CVariableLengthHashTable_Get_Result Get(size_t uiOffset, uint8_t* pValue,
            size_t& valueLength) const;

    CVariableLengthHashTable_Get_Result Get(size_t uiOffset, size_t skipSpaces, uint8_t* pValue,
            size_t& valueLength) const;

    static size_t GetMinimumSize(size_t uiValueUnitSize)
    {
        size_t uiUnitSize = GetAlignedSize(sizeof(CLinkItem)+uiValueUnitSize);
        //至少需要有100个结点吧。
        return GetAlignedSize(sizeof(bool))+GetAlignedSize(sizeof(size_t))
                +GetAlignedSize(sizeof(size_t))+100*uiUnitSize;
    }

    void FreeSpace(size_t uiOffset);

private:

    CLinkItem* Set(size_t uiOffset, const uint8_t* m_pValue, size_t uiValueLength);

    size_t AllocateSpace(size_t uiValueLength);

    size_t GetUnitNumber(size_t uiValueLength)
    {
        return (uiValueLength+m_unitValueSize-1)/m_unitValueSize;
    }

    //将offset转化成相应类型的指针
    template<typename type>
    type* GetPointer(size_t offset) const
    {
        return (type*) ((m_pBaseAddress+offset));
    }

    uint8_t* m_pBaseAddress;

    size_t* m_pFreeLinkHead;

    //free 的单元个数
    size_t* m_pFreeUnitSize;

    //一个单元内最多可以放入的Value的长度(1个value可能被放入多个单元)。
    size_t m_unitValueSize;

};
}

