
#pragma once

#include "utility/basic_types.h"

/*************************************************************/
class CCooling {
private:
protected:
    uint64_t m_uBeginTick; /*��ȴ��ʼʱ��*/
    uint64_t m_uEndTick; /*��ȴ����ʱ��*/
    uint8_t m_state; //״̬
public:
    CCooling();

    virtual ~CCooling() { }

public:
    void clearCool();

    /*--->[ �����ȴʱ�� ]*/
    uint64_t getCoolTick();

    uint64_t getPassTick();

    /*--->[ �������ȴʱ�� ]*/
    uint64_t getTotalTick();

    /*--->[ ��ʼ��ȴ ]*/
    bool beginCooling(uint64_t uTick);

    bool isTimeOut();

    void setState(uint8_t state);

    uint8_t getState();
};
