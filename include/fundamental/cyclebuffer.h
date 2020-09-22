
#pragma once

#include <assert.h>
#include <strings.h>
#include <vector>

#include "fundamental/common.h"
#include "utility/noncopyable.hpp"

namespace svrlib {
#pragma pack(1)

template<typename T>
struct CycleBufferNode {
  T             data;
  volatile bool isUsed;

  CycleBufferNode(bool bIsUsed = true)
          :
          isUsed(bIsUsed)
  {
  }
};

#pragma pack()

/**
 * 内部支撑类，不希望用户直接使用
 */
template<typename DATA>
class CInternalCycleBufferShm : private noncopyable {
public:
    typedef CycleBufferNode<DATA> NODE;

    /**
     * 第一次使用时pData必须已经清零
     */
    CInternalCycleBufferShm(uint32_t uiMaxCount, uint8_t* pData)
            :
            m_uiMaxCount(uiMaxCount),
            m_uiWriteIndex(reinterpret_cast<uint32_t*> (pData)),
            m_uiReadIndex(m_uiWriteIndex+1),
            m_pNodes(reinterpret_cast<NODE*> (m_uiReadIndex+1))
    {

    }

    ~CInternalCycleBufferShm()
    {

    }

    bool Write(const DATA& data)
    {
        if (*m_uiWriteIndex>=m_uiMaxCount || *m_uiWriteIndex<0)
        {
            return false;

        }
        if ((m_pNodes[*m_uiWriteIndex].isUsed))
        {
            return false;

        }

        m_pNodes[*m_uiWriteIndex].data   = data;
        m_pNodes[*m_uiWriteIndex].isUsed = true;

        if (*m_uiWriteIndex<m_uiMaxCount-1)
        {
            ++*m_uiWriteIndex;
        }
        else
        {
            *m_uiWriteIndex = 0;
        }

        return true;
    }

    bool Read(DATA& data)
    {
        if (*m_uiReadIndex>=m_uiMaxCount || *m_uiReadIndex<0)
        {
            return false;
        }
        if (!m_pNodes[*m_uiReadIndex].isUsed)
        {
            return false;

        }
        data = m_pNodes[*m_uiReadIndex].data;
        m_pNodes[*m_uiReadIndex].isUsed = false;

        if (*m_uiReadIndex<m_uiMaxCount-1)
        {
            ++*m_uiReadIndex;
        }
        else
        {
            *m_uiReadIndex = 0;
        }

        return true;
    }

    uint32_t Capacity() const
    {
        return m_uiMaxCount;
    }

    uint32_t Size() const
    {
        if (*m_uiWriteIndex>*m_uiReadIndex)
        {
            return *m_uiWriteIndex-*m_uiReadIndex;
        }
        else if (*m_uiWriteIndex==*m_uiReadIndex)
        {
            if (*m_uiReadIndex>=0 && *m_uiReadIndex<m_uiMaxCount)
            {
                if (!m_pNodes[*m_uiReadIndex].isUsed)
                {
                    return 0;
                }
                else
                {
                    return m_uiMaxCount;
                }
            }
            else
            {
                return 0;
            }
        }
        else
        {
            return m_uiMaxCount-*m_uiReadIndex+*m_uiWriteIndex;
        }
    }

    static size_t GetNeededMemorySize(uint32_t uiMaxCount)
    {
        return sizeof(uint32_t)+sizeof(uint32_t)+uiMaxCount*sizeof(NODE);
    }

private:
    const uint32_t m_uiMaxCount;
    uint32_t* m_uiWriteIndex;
    uint32_t* m_uiReadIndex;
    NODE    * m_pNodes;
};

template<typename DATA>
class CycleBuffer : private noncopyable {
public:

    CycleBuffer(int32_t uiMaxCount = 1024)
    {
        m_pData = new uint8_t[CInternalCycleBufferShm<DATA>::GetNeededMemorySize(
                uiMaxCount)];
        bzero(m_pData, CInternalCycleBufferShm<DATA>::GetNeededMemorySize(
                uiMaxCount));
        m_pInternalCycleBufferShm = new CInternalCycleBufferShm<DATA>(
                uiMaxCount, m_pData);
    }

    ~CycleBuffer()
    {
        delete m_pInternalCycleBufferShm;
        delete[] m_pData;
    }

    bool Write(const DATA& data)
    {
        return m_pInternalCycleBufferShm->Write(data);
    }

    bool Read(DATA& data)
    {
        return m_pInternalCycleBufferShm->Read(data);
    }

    int Size() const
    {
        return m_pInternalCycleBufferShm->Size();
    }

private:
    uint8_t                      * m_pData;
    CInternalCycleBufferShm<DATA>* m_pInternalCycleBufferShm;
};

} //namespace svrlib


