
#pragma once

#include "utility/basic_types.h"

/*************************************************************/
class CCooling {
private:
protected:
    uint64_t m_uBeginTick; /*冷却开始时间*/
    uint64_t m_uEndTick; /*冷却结束时间*/
    uint8_t m_state; //状态
public:
    CCooling();

    virtual ~CCooling() { }

public:
    void clearCool();

    /*--->[ 获得冷却时间 ]*/
    uint64_t getCoolTick();

    uint64_t getPassTick();

    /*--->[ 获得总冷却时间 ]*/
    uint64_t getTotalTick();

    /*--->[ 开始冷却 ]*/
    bool beginCooling(uint64_t uTick);

    bool isTimeOut();

    void setState(uint8_t state);

    uint8_t getState();
};
