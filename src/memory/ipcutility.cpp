#include "memory/ipcutility.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cassert>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <sys/shm.h>

using namespace std;

namespace svrlib {
CShm::CShm(int iShmKey, size_t iSize, int iFlag)
    : m_pShm(NULL)
    , m_bIsNewCreated(false)
{
    int iOldShmId = shmget(iShmKey, iSize, iFlag & (~IPC_CREAT));
    int iShmId = iOldShmId;
    if (iShmId < 0) {
        iShmId = shmget(iShmKey, iSize, iFlag);
    }
    if (iShmId >= 0) {
        void* pAddress = (uint8_t*)(shmat(iShmId, NULL, 0));
        if (pAddress != reinterpret_cast<void*>(-1)) {
            m_pShm = (uint8_t*)pAddress;
        } else {
            m_pShm = NULL;
        }
    }

    if (iOldShmId < 0 && m_pShm != NULL) {
        m_bIsNewCreated = true;
    }
}

uint8_t* CShm::Get()
{
    return m_pShm;
}

CShm::~CShm()
{
    if (m_pShm != NULL)
        shmdt(m_pShm);
}

int CShm::AttachShm(void*& pShm, int iShmKey, size_t iSize, bool bCreat, int iFlag)
{
    pShm = NULL;
    int iRet = 0;
#ifndef WIN32
    int iShmId = shmget(iShmKey, iSize, iFlag & (~IPC_CREAT));
    if (iShmId < 0 && !bCreat)
        return -1;
    if (iShmId >= 0)
        iRet = 1;
    else {
        iShmId = shmget(iShmKey, iSize, IPC_CREAT | iFlag);
        if (iShmId < 0)
            return -1;
    }

    pShm = shmat(iShmId, NULL, 0);
    if (reinterpret_cast<void*>(-1) == pShm) {
        return -1;
    }
    if (iRet == 0)
        memset(pShm, 0, iSize);
#endif

    return iRet;
}

int CShm::DetachShm(void* pShm)
{
#ifndef WIN32
    if (pShm != NULL)
        return shmdt(pShm);
#endif
    return 0;
}

};
