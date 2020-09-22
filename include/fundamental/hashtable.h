/*
 * 描述：实现HashTable的头文件，采用多阶Hash实现
 */
#pragma once

#include <stdint.h>
#include <cstring>
#include <cassert>
#include <string>
#include <fstream>
#include <cmath>
#include <strings.h>

namespace svrlib {
/**
 * 判断一个32位无符号整数是否是质数
 */
inline bool IsPrime(uint32_t uiNum)
{
    if (uiNum==1 || uiNum==0)
    {
        return false;
    }
    if (uiNum==2 || uiNum==3)
    {
        return true;

    }
    if ((uiNum%2)==0)
    {
        return false;
    }
    uint32_t      uiSqt = (uint32_t) sqrt(uiNum+1);
    for (uint32_t i     = 3; i<=uiSqt; i++)
    {
        if ((uiNum%i)==0)
        {
            return false;
        }
    }
    return true;
}

inline uint32_t ELFHash(const uint8_t* buffer, size_t uiLen)
{
    uint32_t hash = 0;
    uint32_t x    = 0;

    for (size_t i = 0; i<uiLen; i++)
    {
        hash   = (hash << 4)+(buffer[i]);
        if ((x = hash & 0xF0000000L)!=0)
        {
            hash ^= (x >> 24);
            hash &= ~x;
        }

    }
    return (hash & 0x7FFFFFFF);
}

template<typename T>
inline uint32_t GetHashVal(const T& oKey)
{
    return oKey.GetHashValue();
}

template<>
inline uint32_t GetHashVal<unsigned int>(const uint32_t& uiKey)
{
    return uiKey;
}

template<>
inline uint32_t GetHashVal<uint64_t>(const uint64_t& uiKey)
{
    return ELFHash((const uint8_t*) &uiKey, sizeof(uiKey));
}

#pragma pack(1)

template<typename KEY, typename VALUE, bool bIsLRU = false>
class CHashItem {
public:
    CHashItem()
    {
        memset(this, 0, sizeof(CHashItem<KEY, VALUE>));
    }

    bool IsUsed() const
    {
        return m_bIsUsed;
    }

    void SetIsUsed(bool bIsUsed)
    {
        m_bIsUsed = bIsUsed;
    }

    void SetLastAccessIndex(uint32_t uiNowIndex)
    {
        return;
    }

    uint32_t GetLastAccessIndex()
    {
        return 0;
    }

public:
    KEY   m_oKey;
    VALUE m_oValue;
    bool  m_bIsUsed;
};

template<typename KEY, typename VALUE>
class CHashItem<KEY, VALUE, true> {
public:
    CHashItem()
    {
        memset(this, 0, sizeof(CHashItem<KEY, VALUE, true>));
    }

    bool IsUsed() const
    {
        return m_bIsUsed;
    }

    void SetIsUsed(bool bIsUsed)
    {
        m_bIsUsed = bIsUsed;
    }

    void SetLastAccessIndex(uint32_t uiNowIndex)
    {
        m_uiAccessIndex = uiNowIndex;
    }

    uint32_t GetLastAccessIndex()
    {
        return m_uiAccessIndex;
    }

public:
    uint32_t m_uiAccessIndex;
    KEY      m_oKey;
    VALUE    m_oValue;
    bool     m_bIsUsed;
};

#pragma pack()

enum Insert_Result//插入/更新节点结果
{
  INSERT_FAIL              = 0,//更新失败
  INSERT_SUCCESS           = 1,//更新成功
  INSERT_SUCCESS_WILL_FULL = 2,
  //更新成功但是HashTable快满了，调用者应扩容了
  INSERT_SUCCESS_BUT_LRU   = 3
//更新成功但是淘汰了一个老的结点。
};

template<typename KEY>
class SimpleComparator {
public:
    bool IsEqual(const KEY& key1, const KEY& key2) const
    {
        return key1==key2;
    }
};

template<typename KEY, typename VALUE, bool bIsLRU = false, typename Comparator= SimpleComparator<KEY> >
class CHashTable {
    /*enum
    {
        //本实现中采用50阶Hash,对于绝大部分情况是适用的
        HASH_TIME = 50,
    };
    enum
    {
        //本实现中Hash表的最少可以容纳的成员个数。
        MINIMUM_CAPACITY = 500 * HASH_TIME
    };*/
    typedef CHashItem<KEY, VALUE, bIsLRU> ITEMTYPE;
public:
    /**
     * 构造一个HashTable。
     * @param uiMaxElementNumber 可以容纳的成员个数，不能低于500*u32HashTime,否则会抛出异常。本实现
     *     中会自动向上往u32HashTime的整数倍取整。
     * @param pData              HashTable数据块空间的首地址。请注意：
     *     1. HashTable不负责分配或回收这个数据块,但是HashTable假定排他性的使用这块数据块。
     *     2. 这块数据块的大小必须等于CHashTable::GetNeededDataSize(uiMaxElementNumber,u32HashTime)
     *     3. 在第一次由HashTable使用这个数据块前,整个数据块必须清零。
     *     4. 整个数据块可以放在共享内存中，也可以不放在共享内存中。
     *     5. 不能为空指针,否则会抛出异常
     *  @param u32HashTime 可以容忍的hash碰撞次数，默认为50
     *
     */
    CHashTable(uint32_t uiMaxElementNumber, uint8_t* pData,
            uint32_t u32HashTime = 50,
            const Comparator& oComparator = Comparator())
            :
            m_uiMaxElementNumber(GetRoundedUpNumber(uiMaxElementNumber, u32HashTime)),
            m_u32HashTime(u32HashTime),
            MAX_NUMBER_PER_HASH(m_uiMaxElementNumber/u32HashTime),
            m_iterateIndex(0), m_Comparator(oComparator)
    {

        if (m_uiMaxElementNumber<500*u32HashTime)
        {
            throw "The argument  uiMaxElementNumberfor CHashTable is not legal.";
        }
        if (pData==NULL)
        {
            throw "The argument pData is not legal.";
        }

        /**
         * 数据块空间解释：
         * 1.前4个字节表示是否被初始化过
         * 2.然后是目前存放的元素个数
         * 3.HASH_TIME个质数,用于取模。
         * 4.实际元素存放空间
         */
        uint32_t* pbIsInitialized = (uint32_t*) (pData);
        m_pUsedSize = pbIsInitialized+1;

        m_puiPrimes  = m_pUsedSize+1;

        if (bIsLRU)
        {
            m_puiCurrentLRUCounter = (uint32_t*) (pData+CHashTable<KEY,
                                                                   VALUE, true>::GetNeededDataSize(
                    m_uiMaxElementNumber, m_u32HashTime)
                    -sizeof(uint32_t));
        }
        else
        {
            m_puiCurrentLRUCounter = NULL;
        }
        m_pHashItems = reinterpret_cast<ITEMTYPE*> ((m_puiPrimes+u32HashTime));
        if (!(*pbIsInitialized))
        {
            GeneratePrimes();
            *m_pUsedSize     = 0;
            if (bIsLRU)
            {
                *m_puiCurrentLRUCounter = 0;
            }
            *pbIsInitialized = 1;

        }
    }

    ~CHashTable()
    {

    }

    //查找
    bool Get(const KEY& oKey, VALUE& oValue) const
    {
        ITEMTYPE* pElement       = NULL;
        uint32_t      uiIndex;
        uint32_t      uiHash;
        uint32_t      uiHash_key = GetHashVal(oKey);
        //依次对每一阶进行Hash
        for (uint32_t i          = 0; i<m_u32HashTime; ++i)
        {
            uiHash   = uiHash_key%m_puiPrimes[i];
            uiIndex  = i*MAX_NUMBER_PER_HASH+uiHash;
            pElement = &m_pHashItems[uiIndex];

            if (pElement->IsUsed() && m_Comparator.IsEqual(pElement->m_oKey, oKey))
            {
                oValue = pElement->m_oValue;
                if (bIsLRU)
                {
                    pElement->SetLastAccessIndex((*m_puiCurrentLRUCounter)++);
                }
                return true;
            }
        }
        return false;
    }

    //获取oKey对应的偏移地址
    bool GetIndexByKey(const KEY& oKey, uint32_t& u32Index) const
    {
        ITEMTYPE* pElement       = NULL;
        uint32_t      uiIndex;
        uint32_t      uiHash;
        uint32_t      uiHash_key = GetHashVal(oKey);
        //依次对每一阶进行Hash
        for (uint32_t i          = 0; i<m_u32HashTime; ++i)
        {
            uiHash   = uiHash_key%m_puiPrimes[i];
            uiIndex  = i*MAX_NUMBER_PER_HASH+uiHash;
            pElement = &m_pHashItems[uiIndex];

            if (pElement->IsUsed() && m_Comparator.IsEqual(pElement->m_oKey, oKey))
            {
                u32Index = uiIndex;
                return true;
            }
        }
        return false;
    }

    //根据节点的偏移地址,查询得到key和value
    bool GetByIndex(const uint32_t u32Index, KEY& oKey, VALUE& oValue) const
    {
        if (u32Index>=m_uiMaxElementNumber)
        {
            return false;
        }
        if (!m_pHashItems[u32Index].IsUsed())
        {
            return false;
        }
        oKey   = m_pHashItems[u32Index].m_oKey;
        oValue = m_pHashItems[u32Index].m_oValue;
        if (bIsLRU)
        {
            m_pHashItems[u32Index].SetLastAccessIndex(
                    (*m_puiCurrentLRUCounter)++);
        }
        return true;
    }

    //根据Key,查询得到Value的指针
    VALUE* GetValuePtr(const KEY& oKey) const
    {
        uint32_t u32Index = 0;
        if (!GetIndexByKey(oKey, u32Index))
        {
            return NULL;
        }
        if (u32Index>=m_uiMaxElementNumber)
        {
            return NULL;
        }
        if (!m_pHashItems[u32Index].IsUsed())
        {
            return NULL;
        }
        if (bIsLRU)
        {
            m_pHashItems[u32Index].SetLastAccessIndex(
                    (*m_puiCurrentLRUCounter)++);
        }
        return &(m_pHashItems[u32Index].m_oValue);
    }

    //插入或更新
    Insert_Result Insert(const KEY& oKey, const VALUE& oValue)
    {

        bool  bHasOldValue;
        VALUE oldValue;
        return InsertAndGetOldValue(oKey, oValue, bHasOldValue, oldValue);
    }

    //插入或更新
    Insert_Result InsertAndGetOldValue(const KEY& oKey, const VALUE& oValue,
            bool& bHasOldValue, VALUE& oldValue)
    {
        ITEMTYPE* pElement = NULL;
        uint32_t uiBestPosition     = 0;
        bool     bHasFreeSpace      = false;
        uint32_t uiHash_key         = GetHashVal(oKey);
        uint32_t uiMaxLRUDifference = 0;

        bHasOldValue = false;

        //依次对每一阶进行Hash
        for (uint32_t i = 0; i<m_u32HashTime; ++i)
        {
            uint32_t uiHash     = uiHash_key%m_puiPrimes[i];
            uint32_t uiPosition = i*MAX_NUMBER_PER_HASH+uiHash;
            pElement = &m_pHashItems[uiPosition];
            if (pElement->IsUsed())
            {
                if (m_Comparator.IsEqual(pElement->m_oKey, oKey))
                {
                    bHasOldValue = true;
                    oldValue     = pElement->m_oValue;

                    pElement->m_oValue = oValue;
                    if (bIsLRU)
                    {
                        pElement->SetLastAccessIndex((*m_puiCurrentLRUCounter)++);
                    }
                    return INSERT_SUCCESS;
                }
                if (bIsLRU && !bHasFreeSpace)
                {
                    //m_puiCurrentLRUCounter会溢出重新变成0,因此uiLRUDifference本来可能为负值
                    //但是会被uint32_t重新解释为一个很大的整数，不影响结果。
                    uint32_t uiLRUDifference = *m_puiCurrentLRUCounter-pElement->GetLastAccessIndex();
                    //LRU index 差异最大的就是应该被淘汰的。
                    if (uiLRUDifference>uiMaxLRUDifference)
                    {
                        uiMaxLRUDifference = uiLRUDifference;
                        uiBestPosition     = uiPosition;
                    }
                }

            }
            else if (!bHasFreeSpace)
            {
                uiBestPosition = uiPosition;
                bHasFreeSpace  = true;
            }
        }
        if (!bHasFreeSpace && !bIsLRU)
        {
            return INSERT_FAIL;
        }

        pElement = &m_pHashItems[uiBestPosition];
        VALUE* pValue = &(pElement->m_oValue);
        KEY  * pKey   = &(pElement->m_oKey);
        //在这儿pElement->key肯定不等于oKey
        if (pElement->IsUsed())
        {
            bHasOldValue = true;
            oldValue     = pElement->m_oValue;
            pKey->~KEY();
            pValue->~VALUE();
        }

        new(pKey) KEY(oKey);
        new(pValue) VALUE(oValue);

        if (bIsLRU)
        {
            pElement->SetLastAccessIndex((*m_puiCurrentLRUCounter)++);
        }

        if (!pElement->IsUsed())
        {
            bHasOldValue = false;
            pElement->SetIsUsed(true);
            ++*m_pUsedSize;
            if ((double(*m_pUsedSize))/(double(m_uiMaxElementNumber))>=0.6)
            {
                return INSERT_SUCCESS_WILL_FULL;
            }
            return INSERT_SUCCESS;
        }
        return INSERT_SUCCESS_BUT_LRU;

    }

    //删除
    bool DeleteAndGetOldValue(const KEY& oKey, VALUE& oldValue)
    {
        ITEMTYPE* pElement  = NULL;
        uint32_t uiHash;
        uint32_t uiHash_key = GetHashVal(oKey);

        //依次对每一阶进行Hash
        for (uint32_t i = 0; i<m_u32HashTime; ++i)
        {
            uiHash   = uiHash_key%m_puiPrimes[i];
            pElement = &m_pHashItems[i*MAX_NUMBER_PER_HASH+uiHash];
            if (!pElement->IsUsed())
            {
                continue;

            }
            else if (m_Comparator.IsEqual(pElement->m_oKey, oKey))
            {
                oldValue = pElement->m_oValue;

                VALUE* pValue = &(pElement->m_oValue);
                KEY  * pKey   = &(pElement->m_oKey);
                pKey->~KEY();
                pValue->~VALUE();

                pElement->SetIsUsed(false);
                --*m_pUsedSize;
                return true;
            }
        }
        return false;
    }

    //删除
    bool Delete(const KEY& oKey)
    {
        VALUE oldValue;
        return DeleteAndGetOldValue(oKey, oldValue);
    }

    /**
     * 清空全部数据
     */
    void Clear()
    {
        bzero(m_pHashItems, m_uiMaxElementNumber*sizeof(ITEMTYPE));
        *m_pUsedSize = 0;
        m_iterateIndex = 0;
    }

    void BeginIterate()
    {
        m_iterateIndex = 0;
    }

    /**
     * Iterate the hashtable. The function will return NULL when there is no more
     * elements.
     * Note:
     * 1. Before the iteration, BeginIterate() should be called first.
     * 2. The function Dump is implemented with this function, so do not call GetNext()
     *    and Dump() alternatively.
     */
    const CHashItem<KEY, VALUE, bIsLRU>* GetNext()
    {
        while (m_iterateIndex<m_uiMaxElementNumber)
        {
            m_iterateIndex++;
            if (m_pHashItems[m_iterateIndex-1].IsUsed())
            {
                return &m_pHashItems[m_iterateIndex-1];
            }
        }
        return NULL;
    }

    /**
     * Dump 到文件中，每一个数据元素对应于一项,如 key:1 value:2
     */
    bool Dump(const std::string& sFilePath)
    {
        BeginIterate();
        std::ofstream oFstream(sFilePath.c_str());
        const ITEMTYPE* nextItem = NULL;
        while ((nextItem = GetNext())!=NULL)
        {
            oFstream << "key: " << nextItem->m_oKey << " value:"
                     << nextItem->m_oValue << std::endl;
        }
        return oFstream.good();
    }

    /**
     * 共享内存是否由本进程初始化的。
     */
    bool IsFirstInitialized()
    {
        return m_bIsFirstInitialized;
    }

    /**
     * 已使用的空间对应的元素个数。
     */
    uint32_t GetUsedElementSize()
    {
        return *m_pUsedSize;
    }

    /**
     * 计算对于容量为uiMaxElementNumber的HashTable所需的内存空间大小
     */
    static size_t GetNeededDataSize(uint32_t uiMaxElementNumber, uint32_t u32HashTime = 50)
    {
        size_t size = sizeof(uint32_t)+sizeof(uint32_t)+u32HashTime
                *sizeof(uint32_t)+
                GetRoundedUpNumber(uiMaxElementNumber, u32HashTime)
                        *sizeof(ITEMTYPE);
        if (bIsLRU)
        {
            size += sizeof(uint32_t);
        }
        return size;
    }

private:
    void GeneratePrimes()
    {
        uint32_t uiMax = m_uiMaxElementNumber/m_u32HashTime;
        /**
         * uiMax要不小于500的原因：
         * 1. 保证 <=uiMax的整数里有HASH_TIME个质数。
         * 2. 质数大一些，取模/HashTable的效果会好一些。
         */
        assert(uiMax>=500);
        if (uiMax%2==0)
        {
            uiMax -= 1;
        }

        for (uint32_t i = 0, iCurrent = uiMax; i<m_u32HashTime && iCurrent>0; iCurrent -= 2)
        {
            if (IsPrime(iCurrent))
            {
                m_puiPrimes[i++] = iCurrent;
            }
        }

    }

    /**
     * 向上往u32HashTime的倍数去整。
     */
    static uint32_t GetRoundedUpNumber(uint32_t uiNumber, uint32_t u32HashTime)
    {
        return (uiNumber+u32HashTime-1)/u32HashTime*u32HashTime;

    }

private:
    CHashTable(const CHashTable& oHashTable);

    CHashTable& operator=(const CHashTable& oHashTable);

    uint32_t  m_uiMaxElementNumber;
    uint32_t  m_u32HashTime;
    const int MAX_NUMBER_PER_HASH;
    bool      m_bIsFirstInitialized;
    uint32_t* m_pUsedSize;

    uint32_t* m_puiCurrentLRUCounter;
    uint32_t* m_puiPrimes;

    ITEMTYPE* m_pHashItems;
    uint32_t m_iterateIndex;
    const Comparator& m_Comparator;
};
}

