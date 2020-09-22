
#pragma once

#include <string>
#include <sys/ipc.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "fundamental/common.h"

namespace svrlib {

/**
 * <p>共享内存的封装</p>
 * <p>2008-7-2 lex 建立</p>
 * <p>2008-11-3 peterzhao修改</p>
 * <p>2009-11-5 keonxiong修改
 * <p>2011-11-15 toney修改
 */
class CShm {
public:
    /**
            构造函数：创建或attach共享内存,可通过Get()来判断是否失败。如果是第一次创建，空间将会被清零
     @param pShm:指向共享内存
     @param iShmKey:共享内存的key
     @param iSize:共享内存的大小
     @param flag：即使包含IPC_CREAT，如果已存在，也不会新创建，只有不存在，才会新创建。
     */
    CShm(int iShmKey, size_t iSize, int iFlag = 0666 | IPC_CREAT);

    /**
     * @return 共享内存地址, NULL表示得到共享内存失败
     */
    uint8_t* Get();

    /**
     * 是否新创建
     */
    bool IsNew()
    {
        return m_bIsNewCreated;
    }

    /**
     * 析构函数，会detach共享内存
     */
    ~CShm();

    static int AttachShm(void*& pShm, int iShmKey, size_t iSize, bool bCreat = true, int iFlag = 0666);

    static int DetachShm(void* pShm);

private:
    uint8_t* m_pShm;
    bool m_bIsNewCreated;
};

class CSemaphore {
public:
    /**
     * 构造信号量，可通过IsValid()来判断是否构造成功。
     * @sName 信号量名字
     * @uiValue 可同时操纵此信号量的进程个数。
     */
    CSemaphore(const std::string& sName, unsigned int uiValue)
            :
            m_pstSem(SEM_FAILED)
    {
        m_pstSem = sem_open(sName.c_str(), O_CREAT, 0666, uiValue);
    }

    bool Wait()
    {
        if (m_pstSem==SEM_FAILED)
        {
            return false;
        }
        return sem_wait(m_pstSem)==0;
    }

    bool Post()
    {
        if (m_pstSem==SEM_FAILED)
        {
            return false;
        }
        return sem_post(m_pstSem)==0;

    }

    bool IsValid()
    {
        return m_pstSem!=SEM_FAILED;
    }

    ~CSemaphore()
    {
        if (m_pstSem!=SEM_FAILED)
        {
            sem_close(m_pstSem);
        }
    }

private:
    CSemaphore(const CSemaphore& oSemaphore);

    CSemaphore& operator=(const CSemaphore& oSemaphore);

    sem_t* m_pstSem;
};

}


