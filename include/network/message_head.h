
#pragma once

#include "crypt/CRC.h"
#include "time/time.hpp"

namespace Network {
    const static uint32_t MAX_MESSAGE_LENGTH = 64 * 1024;//网络层限制最大包长
    class MsgDecode
    {
    public:
        //0收包不完全，>0 获取收到的包长，-1 错误包
        virtual int GetPacketLen(const char * pData, uint32_t len){ return len; }
    };
};

