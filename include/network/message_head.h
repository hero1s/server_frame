
#pragma once

#include "crypt/CRC.h"
#include "time/time.hpp"

namespace Network {
    const static uint32_t MAX_MESSAGE_LENGTH = 64 * 1024;//���������������
    class MsgDecode
    {
    public:
        //0�հ�����ȫ��>0 ��ȡ�յ��İ�����-1 �����
        virtual int GetPacketLen(const char * pData, uint32_t len){ return len; }
    };
};

