#include "memory/memory_pools.h"

namespace Network {
    CMemoryPools *CMemoryPools::m_pMemoryPools = NULL;

    CMemoryPools::CMemoryPools(void) {
        Init();
    }

    CMemoryPools::~CMemoryPools(void) {
        Close();
    }

    void CMemoryPools::Init() {
        m_pMemoryList = NULL;
        m_pMemoryListLast = NULL;
    }

    void CMemoryPools::Close() {
        //����̰߳�ȫ
        std::unique_lock<std::mutex> lock(m_mutex);

        //ɾ�������Ѿ�������ļ���
        _MemoryList *pCurrMemoryList = m_pMemoryList;
        while (NULL != pCurrMemoryList) {
            _MemoryBlock *pMemoryUsed = pCurrMemoryList->m_pMemoryUsed;
            while (NULL != pMemoryUsed) {
                if (NULL != pMemoryUsed->m_pBrick) {
                    free(pMemoryUsed->m_pBrick);
                    pMemoryUsed->m_pBrick = NULL;
                }

                pMemoryUsed = pMemoryUsed->m_pNext;
            }

            _MemoryBlock *pMemoryFree = pCurrMemoryList->m_pMemoryFree;
            while (NULL != pMemoryFree) {
                if (NULL != pMemoryFree->m_pBrick) {
                    free(pMemoryFree->m_pBrick);
                    pMemoryFree->m_pBrick = NULL;
                }

                pMemoryFree = pMemoryFree->m_pNext;
            }

            pCurrMemoryList = pCurrMemoryList->m_pMemLNext;
        }
    }

    void *CMemoryPools::SetMemoryHead(void *pBuff, _MemoryList *pList, _MemoryBlock *pBlock) {
        //����ڴ��ͷ
        if (NULL == pBuff) {
            return NULL;
        }

        //��Ϊһ��long��4���ֽڣ���linux��windows�¶���һ���ġ����Լ�������12��
        UINT32 *plData = (UINT32 *) pBuff;

        plData[0] = (UINT32) pList;         //�ڴ������׵�ַ
        plData[1] = (UINT32) pBlock;        //��������ĵ�ַ
        plData[2] = (UINT32) MAGIC_CODE;    //��֤��

        return &plData[3];
    }

    void *CMemoryPools::GetMemoryHead(void *pBuff) {
        if (NULL == pBuff) {
            return NULL;
        }

        long *plData = (long *) pBuff;
        return &plData[3];
    }

    bool CMemoryPools::GetHeadMemoryBlock(void *pBuff, _MemoryList *&pList, _MemoryBlock *&pBlock) {
        char *szbuf = (char *) pBuff;
        UINT32 *plData = (UINT32 *) (szbuf - MAX_MEMORYHEAD_SIZE);
        if (plData[2] != (long) MAGIC_CODE) {
            return false;
        } else {
            pList = (_MemoryList *) plData[0];   //�ڴ������׵�ַ
            pBlock = (_MemoryBlock *) plData[1];  //��������ĵ�ַ

            return true;
        }

    }

    void *CMemoryPools::GetBuff(size_t szBuffSize) {
        //����̰߳�ȫ
        std::unique_lock<std::mutex> lock(m_mutex);

        void *pBuff = NULL;

        //�ж�����ڴ���С�Ƿ���ڡ�
        if (NULL == m_pMemoryList) {
            //��һ��ʹ���ڴ������
            pBuff = malloc(szBuffSize + MAX_MEMORYHEAD_SIZE);
            if (NULL == pBuff) {
                //printf_s("[CMemoryPools::GetBuff] pBuff malloc = NULL.\n");
                return NULL;
            }

            m_pMemoryList = (_MemoryList *) malloc(sizeof(_MemoryList));
            if (NULL == m_pMemoryList) {
                //printf_s("[CMemoryPools::GetBuff] m_pMemoryList new = NULL.\n");
                free(pBuff);
                return NULL;
            }
            m_pMemoryList->Init();

            //�½�һ���ڴ������
            //_MemoryBlock* pMemoryUsed = new _MemoryBlock();
            _MemoryBlock *pMemoryUsed = (_MemoryBlock *) malloc(sizeof(_MemoryBlock));
            if (NULL == pMemoryUsed) {
                //printf_s("[CMemoryPools::GetBuff] pMemoryBrick new = NULL.\n");
                free(pBuff);
                return NULL;
            }
            pMemoryUsed->Init();

            pMemoryUsed->m_pBrick = pBuff;

            m_pMemoryList->m_nSize = (int) szBuffSize;
            m_pMemoryList->m_pMemoryUsed = pMemoryUsed;
            m_pMemoryList->m_pMemoryUsedLast = pMemoryUsed;

            m_pMemoryListLast = m_pMemoryList;

            //return pBuff;
            return SetMemoryHead(pBuff, m_pMemoryList, pMemoryUsed);
        }

        //�������е��������Ƿ���ڿ����ڴ��
        _MemoryList *pCurrMemoryList = m_pMemoryList;
        while (NULL != pCurrMemoryList) {
            if (pCurrMemoryList->m_nSize == (int) szBuffSize) {
                //����ҵ�������������������������Ѱ�����ɵ��ڴ��
                _MemoryBlock *pMemoryFree = pCurrMemoryList->m_pMemoryFree;
                if (NULL == pMemoryFree) {
                    //û��ʣ��������ڴ�飬�½��ڴ�顣
                    pBuff = malloc(szBuffSize + MAX_MEMORYHEAD_SIZE);
                    if (NULL == pBuff) {
                        //printf_s("[CMemoryPools::GetBuff] (pMemoryFree) pBuff malloc = NULL.\n");
                        return NULL;
                    }

                    //�½�һ���ڴ������
                    //_MemoryBlock* pMemoryUsed = new _MemoryBlock();
                    _MemoryBlock *pMemoryUsed = (_MemoryBlock *) malloc(sizeof(_MemoryBlock));
                    if (NULL == pMemoryUsed) {
                        //printf_s("[CMemoryPools::GetBuff] pMemoryBrick new = NULL.\n");
                        free(pBuff);
                        return NULL;
                    }
                    pMemoryUsed->Init();

                    pMemoryUsed->m_pBrick = pBuff;
                    _MemoryBlock *pMemoryUsedLast = m_pMemoryList->m_pMemoryUsedLast;
                    if (NULL == pMemoryUsedLast) {
                        //printf_s("[CMemoryPools::GetBuff] û�����ʹ�õ��ڴ� pBuff = 0x%08x.\n", pBuff);
                        pCurrMemoryList->m_nSize = (int) szBuffSize;
                        pCurrMemoryList->m_pMemoryUsed = pMemoryUsed;
                        pCurrMemoryList->m_pMemoryUsedLast = pMemoryUsed;
                        //return pBuff;
                        return SetMemoryHead(pBuff, pCurrMemoryList, pMemoryUsed);
                    } else {
                        pMemoryUsed->m_pPrev = pCurrMemoryList->m_pMemoryUsedLast;
                        pCurrMemoryList->m_pMemoryUsedLast->m_pNext = pMemoryUsed;
                        pCurrMemoryList->m_pMemoryUsedLast = pMemoryUsed;
                        //return pBuff;
                        return SetMemoryHead(pBuff, pCurrMemoryList, pMemoryUsed);
                    }
                } else {
                    //�������ڴ浯����
                    _MemoryBlock *pMemoryTemp = pMemoryFree;
                    pCurrMemoryList->m_pMemoryFree = pMemoryFree->m_pNext;
                    pBuff = pMemoryTemp->m_pBrick;

                    pMemoryTemp->m_pPrev = pCurrMemoryList->m_pMemoryUsedLast;
                    pMemoryFree->m_pNext = NULL;

                    if (NULL == pCurrMemoryList->m_pMemoryUsedLast) {
                        pCurrMemoryList->m_pMemoryUsedLast = pMemoryTemp;
                        pCurrMemoryList->m_pMemoryUsed = pMemoryTemp;
                    } else {
                        pCurrMemoryList->m_pMemoryUsedLast->m_pNext = pMemoryTemp;
                        pCurrMemoryList->m_pMemoryUsedLast = pMemoryTemp;
                    }

                    //printf_s("[CMemoryPools::GetBuff] �����ڴ浯���� pBuff = 0x%08x.\n", pBuff);
                    //return pBuff;
                    return GetMemoryHead(pBuff);
                }
            } else {
                pCurrMemoryList = pCurrMemoryList->m_pMemLNext;
            }
        }

        //��������ڴ��в����������ڴ�飬���½�һ��Memorylist

        //û��ʣ��������ڴ�飬�½��ڴ�顣
        pBuff = malloc(szBuffSize + MAX_MEMORYHEAD_SIZE);
        if (NULL == pBuff) {
            //printf_s("[CMemoryPools::GetBuff] (m_pMemoryList) pBuff malloc = NULL.\n");
            return NULL;
        }

        //_MemoryList* pMemoryList = new _MemoryList();
        _MemoryList *pMemoryList = (_MemoryList *) malloc(sizeof(_MemoryList));
        if (NULL == pMemoryList) {
            //printf_s("[CMemoryPools::GetBuff] (m_pMemoryList) m_pMemoryList new = NULL.\n");
            free(pBuff);
            return NULL;
        }
        pMemoryList->Init();

        //�½�һ���ڴ������
        //_MemoryBlock* pMemoryUsed = new _MemoryBlock();
        _MemoryBlock *pMemoryUsed = (_MemoryBlock *) malloc(sizeof(_MemoryBlock));
        if (NULL == pMemoryUsed) {
            //printf_s("[CMemoryPools::GetBuff] (m_pMemoryList) pMemoryBrick new = NULL.\n");
            free(pBuff);
            return NULL;
        }
        pMemoryUsed->Init();

        pMemoryUsed->m_pBrick = pBuff;

        pMemoryList->m_nSize = (int) szBuffSize;
        pMemoryList->m_pMemoryUsed = pMemoryUsed;
        pMemoryList->m_pMemoryUsedLast = pMemoryUsed;

        m_pMemoryListLast->m_pMemLNext = pMemoryList;
        m_pMemoryListLast = pMemoryList;

        //return pBuff;
        return SetMemoryHead(pBuff, pMemoryList, pMemoryUsed);
    }

    bool CMemoryPools::DelBuff(size_t szBuffSize, void *pBuff) {
        //����̰߳�ȫ
        std::unique_lock<std::mutex> lock(m_mutex);

        //���ڴ����Ѱ��ָ���ĵ�ַ���Ƿ���ڣ�������ڱ��Ϊ�Ѿ��ͷš�
        _MemoryList *pCurrMemoryList = m_pMemoryList;
        while (NULL != pCurrMemoryList) {
            if (pCurrMemoryList->m_nSize == (int) szBuffSize) {
                _MemoryBlock *pMemoryUsed = pCurrMemoryList->m_pMemoryUsed;

                bool blFirst = true;
                while (NULL != pMemoryUsed) {
                    if (pBuff == pMemoryUsed->m_pBrick) {
                        if (NULL != pMemoryUsed) {
                            //������б�ĵ�һ������ֱ�Ӱ���һ������ĵ�ַ���Ƶ�ǰһ��
                            if (true == blFirst) {
                                //��ָ��ǰ�˵ĵ�ַ��ֵΪ��ǰָ��֮ǰ��ָ��
                                if (NULL != pMemoryUsed->m_pNext) {
                                    pMemoryUsed->m_pNext->m_pPrev = pMemoryUsed->m_pPrev;
                                }

                                pCurrMemoryList->m_pMemoryUsed = pMemoryUsed->m_pNext;

                                if (pMemoryUsed == pCurrMemoryList->m_pMemoryUsedLast) {
                                    pCurrMemoryList->m_pMemoryUsedLast = pCurrMemoryList->m_pMemoryUsedLast->m_pPrev;
                                }

                                blFirst = false;
                            } else {
                                //��ָ��ǰ�˵ĵ�ַ��ֵΪ��ǰָ��֮ǰ��ָ��
                                if (NULL != pMemoryUsed->m_pNext) {
                                    pMemoryUsed->m_pNext->m_pPrev = pMemoryUsed->m_pPrev;
                                }

                                if (pMemoryUsed == pCurrMemoryList->m_pMemoryUsedLast) {
                                    pCurrMemoryList->m_pMemoryUsedLast = pCurrMemoryList->m_pMemoryUsedLast->m_pPrev;
                                } else {
                                    pMemoryUsed->m_pPrev->m_pNext = pMemoryUsed->m_pNext;
                                    //printf_s("[CMemoryPools::DelBuff] �ڴ�ָ��ָ����һ�� pMemoryUsed->m_pPrev->m_pNext = 0x%08x.\n", pMemoryUsed->m_pPrev->m_pNext->m_pBrick);
                                }
                            }

                            if (pCurrMemoryList->m_pMemoryFree == NULL) {
                                //printf_s("[CMemoryPools::DelBuff] pBuff = 0x%08x.\n", pBuff);
                                pMemoryUsed->m_pPrev = NULL;
                                pMemoryUsed->m_pNext = NULL;
                                pCurrMemoryList->m_pMemoryFree = pMemoryUsed;
                                pCurrMemoryList->m_pMemoryFreeLast = pMemoryUsed;

                            } else {
                                //printf_s("[CMemoryPools::DelBuff] pBuff = 0x%08x.\n", pBuff);
                                pMemoryUsed->m_pPrev = pCurrMemoryList->m_pMemoryFreeLast;
                                pMemoryUsed->m_pNext = NULL;
                                pCurrMemoryList->m_pMemoryFreeLast->m_pNext = pMemoryUsed;
                                pCurrMemoryList->m_pMemoryFreeLast = pMemoryUsed;
                            }

                            return true;
                        } else {
                            //printf_s("[CMemoryPools::DelBuff] pBuff = 0x%08x pMemoryUsedProv is NULL.\n", pBuff);
                            return false;
                        }
                    }

                    pMemoryUsed = pMemoryUsed->m_pNext;
                    blFirst = false;
                }
            }

            pCurrMemoryList = pCurrMemoryList->m_pMemLNext;
        }

        //printf_s("[CMemoryPools::DelBuff] pBuff = 0x%08x is not memoryPool.\n", pBuff);
        return false;
    }

    bool CMemoryPools::DelBuff(void *pBuff) {
        //����̰߳�ȫ
        std::unique_lock<std::mutex> lock(m_mutex);

        _MemoryBlock *pMemoryUsed = NULL;
        _MemoryList *pCurrMemoryList = NULL;

        if (false == GetHeadMemoryBlock(pBuff, pCurrMemoryList, pMemoryUsed)) {
            return false;
        }

        if (NULL != pMemoryUsed && NULL != pCurrMemoryList) {
            //������б�ĵ�һ������ֱ�Ӱ���һ������ĵ�ַ���Ƶ�ǰһ��
            if (pCurrMemoryList->m_pMemoryUsed == pMemoryUsed) {
                pCurrMemoryList->m_pMemoryUsed = pMemoryUsed->m_pNext;
            } else {
                pMemoryUsed->m_pPrev->m_pNext = pMemoryUsed->m_pNext;
            }

            if (NULL != pMemoryUsed->m_pNext) {
                pMemoryUsed->m_pNext->m_pPrev = pMemoryUsed->m_pPrev;
            }

            if (pMemoryUsed == pCurrMemoryList->m_pMemoryUsedLast) {
                pCurrMemoryList->m_pMemoryUsedLast = pCurrMemoryList->m_pMemoryUsedLast->m_pPrev;
            }

            if (pCurrMemoryList->m_pMemoryFree == NULL) {
                //printf_s("[CMemoryPools::DelBuff] pBuff = 0x%08x.\n", pBuff);
                pMemoryUsed->m_pPrev = NULL;
                pMemoryUsed->m_pNext = NULL;
                pCurrMemoryList->m_pMemoryFree = pMemoryUsed;
                pCurrMemoryList->m_pMemoryFreeLast = pMemoryUsed;
                //printf_s("[CMemoryPools::DelBuff] �ڴ��б�Ϊ�� m_pMemoryFree.m_pBrick = 0x%08x.\n", pCurrMemoryList->m_pMemoryFreeLast->m_pBrick);
            } else {
                //printf_s("[CMemoryPools::DelBuff] pBuff = 0x%08x.\n", pBuff);
                pMemoryUsed->m_pPrev = pCurrMemoryList->m_pMemoryFreeLast;
                pMemoryUsed->m_pNext = NULL;
                pCurrMemoryList->m_pMemoryFreeLast->m_pNext = pMemoryUsed;
                pCurrMemoryList->m_pMemoryFreeLast = pMemoryUsed;
                //printf_s("[CMemoryPools::DelBuff] �ڴ��б�ǿ� m_pMemoryFree.m_pBrick = 0x%08x.\n", pCurrMemoryList->m_pMemoryFreeLast->m_pBrick);
            }

            return true;
        } else {
            //printf_s("[CMemoryPools::DelBuff] pBuff = 0x%08x pMemoryUsedProv is NULL.\n", pBuff);
            return false;
        }


        //printf_s("[CMemoryPools::DelBuff] pBuff = 0x%08x is not memoryPool.\n", pBuff);
        return false;
    }

    void CMemoryPools::DisplayMemoryList() {
        int nUsedCount = 0;
        int nFreeCount = 0;

        _MemoryList *pCurrMemoryList = m_pMemoryList;
        while (NULL != pCurrMemoryList) {
            _MemoryBlock *pMemoryUsed = pCurrMemoryList->m_pMemoryUsed;
            _MemoryBlock *pMemoryFree = pCurrMemoryList->m_pMemoryFree;

            nUsedCount = 0;
            nFreeCount = 0;

            while (NULL != pMemoryUsed) {
                nUsedCount++;
                pMemoryUsed = pMemoryUsed->m_pNext;
            }
            //printf_s("[CMemoryPools::DisplayMemoryList] pMemoryUsed nUsedCount = %d, Size = %d.\n", nUsedCount, pCurrMemoryList->m_nSize * nUsedCount);

            while (NULL != pMemoryFree) {
                nFreeCount++;
                pMemoryFree = pMemoryFree->m_pNext;
            }
            //printf_s("[CMemoryPools::DisplayMemoryList] pMemoryFree nFreeCount = %d, Size = %d.\n", nFreeCount, pCurrMemoryList->m_nSize * nFreeCount);

            pCurrMemoryList = pCurrMemoryList->m_pMemLNext;
        }
    }

}