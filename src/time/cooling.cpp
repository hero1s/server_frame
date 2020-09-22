
#include "time/time.hpp"
#include "time/cooling.h"
/*************************************************************/
//-------------------------------------------------------------
//------------------------------
using namespace svrlib;

CCooling::CCooling()
{
    clearCool();
    m_state = 0;
}

//-------------------------------------------------------------
//------------------------------ 
void CCooling::clearCool()
{
    m_uBeginTick = 0;
    m_uEndTick   = 0;
}

//-------------------------------------------------------------
//------------------------------ 
uint32_t CCooling::getCoolTick()
{
    if (!m_uBeginTick || !m_uEndTick)
        return 0;

    uint64_t uTick = time::getSystemTick64();
    return (m_uEndTick>uTick ? uint32_t(m_uEndTick-uTick) : 0);
}

uint32_t CCooling::getPassTick()
{
    if (!m_uBeginTick || !m_uEndTick)
        return 0;

    uint64_t uTick = time::getSystemTick64();
    if (uTick>m_uBeginTick)
        return (uint32_t(uTick-m_uBeginTick));
    return 0;
}

//-------------------------------------------------------------
//------------------------------ �������ȴʱ��
uint32_t CCooling::getTotalTick()
{
    return ((m_uEndTick>m_uBeginTick) ? uint32_t(m_uEndTick-m_uBeginTick) : 0);
}

//-------------------------------------------------------------
//------------------------------ 
bool CCooling::beginCooling(uint32_t uTick)
{
    if (uTick<=0)
    {
        m_uBeginTick = 0;
        m_uEndTick   = 0;
        return false;
    }
    m_uBeginTick = time::getSystemTick64();
    m_uEndTick   = m_uBeginTick+uTick;

    return true;
}

bool CCooling::isTimeOut()
{
    return getCoolTick()==0;
}

void CCooling::setState(uint8_t state)
{
    m_state = state;
}

uint8_t CCooling::getState()
{
    return m_state;
}


















