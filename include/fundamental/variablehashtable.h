
#pragma once

#include <stdint.h>
#include <cstring>
#include <cassert>
#include <string>
#include <fstream>
#include <cmath>
#include <strings.h>
#include "fundamental/hashtable.h"
#include "fundamental/linkcache.h"
#include "memory/ipcutility.h"
#include "linkcache.h"


namespace svrlib {

/**
 * 变长HashTable.本实现中每一个Value采用一个链表存放，链表中的每个结点长度固定。
 * 因此将共享内存分为两块：
 * 1.定长的HashTable,存放key和对应value链表的首地址
 * 2.其余部分划分成等长的结点，开始时形成一个空闲链表。
 */
template<bool bIsLRU = false>
class CVariableHashTable {
    struct RealKey {
    public:
        uint32_t u32HashKey;
        size_t   uiKeyLength;

        uint32_t GetHashValue() const
        {
            return u32HashKey;
        }

    };

    struct RealValue {
    public:

        size_t memoryLocation;
    };

    class RealKeyComparator {
    public:
        RealKeyComparator(const CLinkCache* pLinkCache,
                size_t maxKeyValueLength)
                :
                m_pLinkCache(pLinkCache), m_uiMaxKeyValueLength(maxKeyValueLength)
        {
            m_pmaxKeyValueBuffer = new uint8_t[maxKeyValueLength];

        }

        void SetLastKey(const uint8_t* pKey)
        {
            m_pLastKey = pKey;
        }

        bool IsEqual(const RealKey& key1, const RealKey& key2) const
        {
            const RealKey* keyInHashtable    = &key1;
            const RealKey* keyNotInHashtable = &key2;

            if (keyInHashtable->u32HashKey!=keyNotInHashtable->u32HashKey
                    || keyInHashtable->uiKeyLength>m_uiMaxKeyValueLength
                    || keyInHashtable->uiKeyLength
                            !=keyNotInHashtable->uiKeyLength)
            {
                return false;
            }

            CHashItem<RealKey, RealValue>* pHashItem = (CHashItem<RealKey,
                                                                  RealValue>*) keyInHashtable;
            size_t uiLength = m_uiMaxKeyValueLength;
            if (m_pLinkCache->Get(pHashItem->m_oValue.memoryLocation, m_pmaxKeyValueBuffer,
                    uiLength)!=GET_SUCCESS)
            {
                return false;
            }

            if (uiLength<keyInHashtable->uiKeyLength)
            {
                return false;
            }

            return strncmp((const char*) m_pmaxKeyValueBuffer, (const char*) m_pLastKey,
                    keyInHashtable->uiKeyLength)==0;

        }

        ~RealKeyComparator()
        {

            delete[] m_pmaxKeyValueBuffer;
        }

    private:
        const CLinkCache* m_pLinkCache;
        const uint8_t   * m_pLastKey;
        size_t m_uiMaxKeyValueLength;
        uint8_t* m_pmaxKeyValueBuffer;
    };

public:
    /**
     * 构造一个共享内存中的变长HashTable。
     * @param iShmKey 共享内存的key
     * @param uiMemorySize 共享内存的大小,不能太小，否则会抛出异常
     * @param uiMaxElementNumber 可以容纳的成员个数，不能低于500*50,否则会抛出异常。本实现
     *     中会自动向上往50的整数倍取整。
     * @param uiUnitValueSize 每个结点/单元中存放的value长度,为了对齐，实际长度可能稍大。
     */
    CVariableHashTable(int iShmKey, size_t uiMemorySize,
            uint32_t u32MaxElementNumber, uint32_t u32UnitValueSize,
            size_t u32MaxkeyvalueLength = 64*1024)
    {
        size_t uiHashTableSize =
                       GetAlignedSize(
                               CHashTable<RealKey, RealValue, bIsLRU,
                                          RealKeyComparator>::GetNeededDataSize(
                                       u32MaxElementNumber));

        if (uiMemorySize<uiHashTableSize+CLinkCache::GetMinimumSize(
                u32UnitValueSize))
        {
            throw "The memory size is too small!";
        }

        m_pShm = new CShm(iShmKey, uiMemorySize);
        if (m_pShm->Get()==NULL)
        {
            throw "The shared memory created/attached fails!";
        }
        m_pLinkCache = new CLinkCache(m_pShm->Get()+uiHashTableSize,
                uiMemorySize-uiHashTableSize, u32UnitValueSize);

        m_prealKeyComparator = new RealKeyComparator(m_pLinkCache,
                u32MaxkeyvalueLength);

        m_pHashTable = new CHashTable<RealKey, RealValue, bIsLRU,
                                      RealKeyComparator>(u32MaxElementNumber, m_pShm->Get(),
                *m_prealKeyComparator);

    }

    ~CVariableHashTable()
    {
        delete m_pLinkCache;
        delete m_pHashTable;
        delete m_pShm;
        delete m_prealKeyComparator;

    }

    Insert_Result Insert(const uint8_t* pKey, size_t uiKeyLength,
            const uint8_t* m_pValue, size_t uiValueLength)
    {
        m_prealKeyComparator->SetLastKey(pKey);
        size_t uiAllocatedOffset = m_pLinkCache->Set(pKey, uiKeyLength,
                m_pValue, uiValueLength);
        if (uiAllocatedOffset==0)
        {
            return INSERT_FAIL;
        }

        RealKey oKey;

        oKey.u32HashKey  = ELFHash(pKey, uiKeyLength);
        oKey.uiKeyLength = uiKeyLength;

        RealValue value;
        value.memoryLocation = uiAllocatedOffset;

        bool          bHasOldValue;
        RealValue     uiOldValue;
        Insert_Result result = m_pHashTable->InsertAndGetOldValue(oKey,
                value, bHasOldValue, uiOldValue);
        if (result==INSERT_FAIL)
        {
            m_pLinkCache->FreeSpace(uiAllocatedOffset);
        }
        else if (bHasOldValue)
        {
            m_pLinkCache->FreeSpace(uiOldValue.memoryLocation);
        }

        return result;
    }

    bool Delete(const uint8_t* pKey, size_t uiKeyLength)
    {
        m_prealKeyComparator->SetLastKey(pKey);

        RealKey oKey;
        oKey.u32HashKey   = ELFHash(pKey, uiKeyLength);
        oKey.uiKeyLength  = uiKeyLength;

        RealValue uiOldValue;
        bool      bResult = m_pHashTable->DeleteAndGetOldValue(oKey, uiOldValue);
        if (bResult)
        {
            m_pLinkCache->FreeSpace(uiOldValue.memoryLocation);
        }
        return bResult;
    }

    //查找
    CVariableLengthHashTable_Get_Result Get(const uint8_t* pKey,
            size_t uiKeyLength, uint8_t* pValue, size_t& valueLength) const
    {

        m_prealKeyComparator->SetLastKey(pKey);
        RealKey oKey;
        oKey.u32HashKey   = ELFHash(pKey, uiKeyLength);
        oKey.uiKeyLength  = uiKeyLength;

        RealValue oValue;
        bool      bResult = m_pHashTable->Get(oKey, oValue);
        if (!bResult)
        {
            return ITEM_DOES_NOT_EXIST;
        }
        CVariableLengthHashTable_Get_Result returnResult = m_pLinkCache->Get(
                oValue.memoryLocation, uiKeyLength, pValue, valueLength);
        return returnResult;
    }

private:

    CVariableHashTable(const CVariableHashTable& oHashTable);

    CVariableHashTable& operator=(const CVariableHashTable& oHashTable);

    CShm* m_pShm;

    //一个value由一个链表存放，Hash表只存放key和链表首地址
    CHashTable<RealKey, RealValue, bIsLRU, RealKeyComparator>* m_pHashTable;

    CLinkCache       * m_pLinkCache;
    RealKeyComparator* m_prealKeyComparator;

};

typedef CVariableHashTable<> CNoLRUVariableHashTable;
}

