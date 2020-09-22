
#pragma once

#include <vector>

using namespace std;

template<typename T>
class CRank {
protected:
    vector<T> m_vecRank;
public:
    CRank()
    {
    }

    virtual ~CRank()
    {
    }

    virtual bool IsSame(T&, T&) const
    {
        return false;
    }

    // a > b, 返回大于0,　　a < b, 返回小于0
    virtual int CmpFunc(T& a, T& b) const
    {
        return 0;
    }

    int GetRank(T& tValue)
    {
        for (int i = 0; i<m_vecRank.size(); ++i)
        {
            if (IsSame(tValue, m_vecRank[i]))
            {
                return i;
            }
        }

        return -1;
    }

    //第一名传入1
    int GetRankByOder(int iOrder, T& tValue)
    {
        if (iOrder>(int) m_vecRank.size() || iOrder<=0)
        {
            return -1;
        }
        tValue = m_vecRank[iOrder-1];
        return 0;
    }

    /*
    更新以CTArray为基础的排名榜 支持数值正负变化
    声明一个CRankArray实例时候需要在cpp文件中实现IsSame和CmpFunc
    iOld 表示更新前的排名 iNew 表示更新后的排名 都从0开始 -1表示不在榜中
    返回值：0
    */
    int Update(T& t, int& iOld, int& iNew)
    {
        //支持数值正负变化后的更新

        bool bAdd  = true;
        bool bSwap = false;

        iOld = -1;
        iNew = -1;

        T stSwap = t;
        T stTemp = t;

        for (int i = 0; i<(int) m_vecRank.size(); ++i)
        {
            if ((!bSwap && CmpFunc(stSwap, m_vecRank[i])>0) ||
                    (bSwap && CmpFunc(stSwap, m_vecRank[i])>=0))
            {
                if (bSwap)
                {
                    if (IsSame(t, m_vecRank[i]))//单元已在榜中但排名变前
                    {
                        bAdd = false;
                        iOld = i;
                        m_vecRank[i] = stSwap;
                        break;
                    }
                    else
                    {
                        stTemp = m_vecRank[i];
                        m_vecRank[i] = stSwap;
                        stSwap = stTemp;
                    }
                }
                else
                {
                    if (IsSame(t, m_vecRank[i]))//单元已在榜中但排名没变
                    {
                        bAdd = false;
                        iOld = i;
                        iNew = i;
                        m_vecRank[i] = stSwap;
                        break;
                    }
                    else
                    {
                        bSwap  = true;
                        iNew   = i;
                        stTemp = m_vecRank[i];
                        m_vecRank[i] = stSwap;
                        stSwap = stTemp;
                    }
                }
            }
            else if (IsSame(t, m_vecRank[i]))//单元已在榜中但排名变后
            {
                bAdd = false;
                iOld = i;
                m_vecRank[i] = stSwap;

                for (; i+1<(int) m_vecRank.size(); ++i)
                {
                    if (CmpFunc(m_vecRank[i], m_vecRank[i+1])<0)
                    {
                        stTemp = m_vecRank[i];
                        m_vecRank[i]   = m_vecRank[i+1];
                        m_vecRank[i+1] = stTemp;
                        iNew = i+1;
                    }
                    else
                    {
                        break;
                    }
                }

                break;
            }
        }

        if (bAdd)
        {
            if (iNew>=0)
            {
                m_vecRank.push_back(stSwap);
            }
            else
            {
                m_vecRank.push_back(stSwap);
                iNew = m_vecRank.size();
            }
        }

        return 0;
    }

    int Update(T& t)
    {
        int iTmp = 0;
        return Update(t, iTmp, iTmp);
    }

    void Clear()
    {
        m_vecRank.clear();
    }
};




