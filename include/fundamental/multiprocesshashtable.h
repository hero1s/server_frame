/*
 * 描述：适用于多进程的共享内存HashTable实现。
 */
#pragma once

#include "fundamental/hashtable.h"
#include "memory/ipcutility.h"

namespace svrlib {
/**
 * CMultiProcessHashTable定义，使用者必须先调用
 * InitShm()后才能调用实际操作函数Get/Insert/Delete/Dump。
 */
template<typename KEY, typename VALUE, bool bIsLRU = false>
class CMultiProcessHashTable {
public:
    CMultiProcessHashTable()
            :m_pShm(NULL), m_pHashTable(NULL)
    {
    }

    /**
     * 初始化共享内存,此共享内存会被重用(已创建）或创建(未创建)
     * @param iShmKey 共享内存的key
     * @param uiMaxElementNumber HashTable的容量，不能低于500*50,否则会抛出异常。本实现
     *     中会自动向上往50的整数倍取整。
     */
    bool InitShm(int iShmKey, uint32_t uiMaxElementNumber, uint32_t u32HashTime = 50)
    {
        m_pShm = new CShm(iShmKey,
                CHashTable<KEY, VALUE, bIsLRU>::GetNeededDataSize(uiMaxElementNumber,
                        u32HashTime));
        if (m_pShm->Get()==NULL)
        {
            return false;
        }
        m_pHashTable = new CHashTable<KEY, VALUE, bIsLRU>(uiMaxElementNumber,
                m_pShm->Get());
        return true;
    }

    bool Get(const KEY& oKey, VALUE& oValue) const
    {
        if (!IsValid())
        {
            return false;
        }
        bool bResult = false;
        bResult = m_pHashTable->Get(oKey, oValue);
        return bResult;
    }

    //获取oKey对应的偏移地址
    bool GetIndexByKey(const KEY& oKey, uint32_t& u32Index) const
    {
        if (!IsValid())
        {
            return false;
        }
        return m_pHashTable->GetIndexByKey(oKey, u32Index);
    }

    //根据节点的偏移地址,查询得到key和value
    bool GetByIndex(const uint32_t u32Index, KEY& oKey, VALUE& oValue) const
    {
        if (!IsValid())
        {
            return false;
        }
        return m_pHashTable->GetByIndex(u32Index, oKey, oValue);
    }

    //根据Key,查询得到Value的指针
    VALUE* GetValuePtr(const KEY& oKey) const
    {
        if (!IsValid())
        {
            return NULL;
        }
        return m_pHashTable->GetValuePtr(oKey);
    }

    Insert_Result Insert(const KEY& oKey, const VALUE& oValue)
    {
        if (!IsValid())
        {
            return INSERT_FAIL;
        }
        Insert_Result eResult;
        eResult = m_pHashTable->Insert(oKey, oValue);
        return eResult;
    }

    bool Delete(const KEY& oKey)
    {
        if (!IsValid())
        {
            return false;
        }
        bool bResult = false;
        bResult = m_pHashTable->Delete(oKey);
        return bResult;
    }

    /**
     * 清空全部数据
     */
    void Clear()
    {
        if (!IsValid())
        {
            return;
        }
        m_pHashTable->Clear();

    }

    void BeginIterate()
    {
        if (!IsValid())
        {
            return;
        }
        m_pHashTable->BeginIterate();
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
        const CHashItem<KEY, VALUE, bIsLRU>* pResult = NULL;
        if (!IsValid())
        {
            return NULL;
        }

        pResult = m_pHashTable->GetNext();
        return pResult;
    }

    /**
     * Dump 到文件中，每一个数据元素对应于一项,如 key:1 value:2
     */
    bool Dump(const std::string& sFilePath)
    {
        if (!IsValid())
        {
            return false;
        }
        bool bResult = false;
        bResult = m_pHashTable->Dump(sFilePath);
        return bResult;
    }

    /**
     * 已使用的空间对应的元素个数。
     */
    uint32_t GetUsedElementSize()
    {
        if (!IsValid())
        {
            return 0;
        }
        return m_pHashTable->GetUsedElementSize();
    }

    bool IsValid() const
    {
        return (m_pShm!=NULL && m_pHashTable!=NULL)
                && (m_pShm->Get()!=NULL);
    }

    ~CMultiProcessHashTable()
    {
        delete m_pShm;
        m_pShm = NULL;
        delete m_pHashTable;
        m_pHashTable = NULL;
    }

private:
    CMultiProcessHashTable(
            const CMultiProcessHashTable& oMultiProcessHashTable);

    CMultiProcessHashTable& operator=(
            const CMultiProcessHashTable& oMultiProcessHashTable);

    CShm                          * m_pShm;
    CHashTable<KEY, VALUE, bIsLRU>* m_pHashTable;

};
}

