
#pragma once

#include "memory/ipcutility.h"
#include "fundamental/cyclebuffer.h"
#include <assert.h>

namespace svrlib {
template<typename DATA>
class CycleBufferShm : private noncopyable {
public:
    CycleBufferShm(int iKey, uint32_t uiMaxCount)
            :
            m_oShm(iKey, CInternalCycleBufferShm<DATA>::GetNeededMemorySize(uiMaxCount)),
            m_pInternalCycleBufferShm(NULL)
    {
        if (m_oShm.Get()!=NULL)
        {
            m_pInternalCycleBufferShm = new CInternalCycleBufferShm<DATA>(uiMaxCount,
                    m_oShm.Get());
        }

    }

    ~CycleBufferShm()
    {
        delete m_pInternalCycleBufferShm;
    }

    bool Write(const DATA& data)
    {
        if (m_pInternalCycleBufferShm==NULL)
        {
            return false;
        }
        return m_pInternalCycleBufferShm->Write(data);
    }

    bool Read(DATA& data)
    {
        if (m_pInternalCycleBufferShm==NULL)
        {
            return false;
        }
        return m_pInternalCycleBufferShm->Read(data);
    }

    uint32_t Size() const
    {
        if (m_pInternalCycleBufferShm==NULL)
        {
            return 0;
        }
        return m_pInternalCycleBufferShm->Size();
    }

private:
    CShm m_oShm;
    CInternalCycleBufferShm<DATA>* m_pInternalCycleBufferShm;
}; //class CycleBufferShm
}
//end namespace


