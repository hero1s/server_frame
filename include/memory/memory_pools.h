#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <mutex>

namespace Network {
#define MAX_MEMORYHEAD_SIZE 12         //4���ֽڵ�ָ����ַ + 4���ֽڵ������׵�ַ + 4λ��֤��
#define MAGIC_CODE          0x123456   //��֤��

#define UINT32 unsigned long

    struct _MemoryBlock  //�ڴ��Ľṹ,˫������
    {
        _MemoryBlock *m_pNext;
        _MemoryBlock *m_pPrev;
        void *m_pBrick;

        void Init() {
            m_pNext = NULL;
            m_pPrev = NULL;
            m_pBrick = NULL;
        };

        _MemoryBlock() {
            Init();
        };
    };

    struct _MemoryList    //�ڴ�����б�
    {
        _MemoryList *m_pMemLNext;
        _MemoryBlock *m_pMemoryFree;       //���ɵ��ڴ��
        _MemoryBlock *m_pMemoryFreeLast;   //���ɵ��ڴ������ĩβ
        _MemoryBlock *m_pMemoryUsed;       //ʹ�õ��ڴ��
        _MemoryBlock *m_pMemoryUsedLast;   //ʹ�õ��ڴ������ĩβ

        int m_nSize;

        void Init() {
            m_pMemLNext = NULL;
            m_pMemoryFree = NULL;
            m_pMemoryUsed = NULL;
            m_pMemoryUsedLast = NULL;
            m_pMemoryFreeLast = NULL;
            m_nSize = 0;
        };

        _MemoryList() {
            Init();
        };
    };

    class CMemoryPools {
    public:
        static CMemoryPools &Instance() {
            if (m_pMemoryPools == NULL) {
                m_pMemoryPools = (CMemoryPools *) malloc(sizeof(CMemoryPools));
                m_pMemoryPools->Init();
            }

            return *m_pMemoryPools;
        }

    public:
        ~CMemoryPools(void);

        void *GetBuff(size_t szBuffSize);

        bool DelBuff(size_t szBuffSize, void *pBuff);

        bool DelBuff(void *pBuff);

        void DisplayMemoryList();

    private:
        CMemoryPools(void);

        void Close();

        void Init();

        void *SetMemoryHead(void *pBuff, _MemoryList *pList, _MemoryBlock *pBlock);

        void *GetMemoryHead(void *pBuff);

        bool GetHeadMemoryBlock(void *pBuff, _MemoryList *&pList, _MemoryBlock *&pBlock);


    private:
        static CMemoryPools *m_pMemoryPools;
        _MemoryList *m_pMemoryList;
        _MemoryList *m_pMemoryListLast;    //���һ���ڴ��������ָ��
        std::mutex   m_mutex;
    };
}