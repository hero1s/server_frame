
#pragma once

#include <stdint.h>
#include <cstring>
#include <cassert>
#include <string>
#include <fstream>
#include <cmath>
#include <strings.h>
#include "fundamental/hashtable.h"
#include "memory/ipcutility.h"
#include "linkcache.h"


namespace svrlib {

/**
 * 变长HashTable.本实现中每一个Value采用一个链表存放，链表中的每个结点长度固定。
 * 因此将共享内存分为两块：
 * 1.定长的HashTable,存放key和对应value链表的首地址
 * 2.其余部分划分成等长的结点，开始时形成一个空闲链表。
 */
template<typename KEY, bool bIsLRU = false>
class CVariableLengthHashTable {

public:
    /**
     * 构造一个共享内存中的变长HashTable。
     * @param iShmKey 共享内存的key
     * @param uiMemorySize 共享内存的大小,不能太小，否则会抛出异常
     * @param uiMaxElementNumber 可以容纳的成员个数，不能低于500*50,否则会抛出异常。本实现
     *     中会自动向上往50的整数倍取整。
     * @param uiUnitValueSize 每个结点/单元中存放的value长度,为了对齐，实际长度可能稍大。
     */
    CVariableLengthHashTable(int iShmKey, size_t uiMemorySize,
            uint32_t uiMaxElementNumber, uint32_t uiUnitValueSize)
    {
        size_t uiHashTableSize = GetAlignedSize(
                CHashTable<KEY, size_t, bIsLRU>::GetNeededDataSize(uiMaxElementNumber));

        if (uiMemorySize<uiHashTableSize+CLinkCache::GetMinimumSize(
                uiUnitValueSize))
        {
            throw "The memory size is too small!";
        }

        m_pShm = new CShm(iShmKey, uiMemorySize);
        if (m_pShm->Get()==NULL)
        {
            throw "The shared memory created/attached fails!";
        }
        m_pHashTable = new CHashTable<KEY, size_t, bIsLRU>(
                uiMaxElementNumber,
                m_pShm->Get());

        m_pLinkCache = new CLinkCache(
                m_pShm->Get()+uiHashTableSize,
                uiMemorySize-uiHashTableSize, uiUnitValueSize);

    }

    ~CVariableLengthHashTable()
    {
        delete m_pLinkCache;
        delete m_pHashTable;
        delete m_pShm;

    }

    Insert_Result Insert(const KEY& oKey, const uint8_t* m_pValue,
            size_t uiValueLength)
    {
        size_t uiAllocatedOffset = m_pLinkCache->Set(m_pValue, uiValueLength);
        if (uiAllocatedOffset==0)
        {
            return INSERT_FAIL;
        }
        bool          bHasOldValue;
        size_t        uiOldValue;
        Insert_Result result     = m_pHashTable->InsertAndGetOldValue(oKey,
                uiAllocatedOffset, bHasOldValue,
                uiOldValue);
        if (result==INSERT_FAIL)
        {
            m_pLinkCache->FreeSpace(uiAllocatedOffset);
        }
        else if (bHasOldValue)
        {
            m_pLinkCache->FreeSpace(uiOldValue);
        }

        return result;
    }

    bool Delete(const KEY& oKey)
    {
        size_t uiOldValue;
        bool   bResult = m_pHashTable->DeleteAndGetOldValue(oKey,
                uiOldValue);
        if (bResult)
        {
            m_pLinkCache->FreeSpace(uiOldValue);
        }
        return bResult;
    }

    //查找
    CVariableLengthHashTable_Get_Result Get(const KEY& oKey, uint8_t* pValue,
            size_t& valueLength) const
    {
        size_t uiOffset;
        bool   bResult = m_pHashTable->Get(oKey, uiOffset);
        if (!bResult)
        {
            return ITEM_DOES_NOT_EXIST;
        }
        return m_pLinkCache->Get(uiOffset, pValue, valueLength);
    }

private:

    CVariableLengthHashTable(const CVariableLengthHashTable& oHashTable);

    CVariableLengthHashTable& operator=(
            const CVariableLengthHashTable& oHashTable);

    CShm* m_pShm;

    //一个value由一个链表存放，Hash表只存放key和链表首地址
    CHashTable<KEY, size_t, bIsLRU>* m_pHashTable;

    CLinkCache* m_pLinkCache;
};
}

