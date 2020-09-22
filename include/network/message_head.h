
#pragma once

#include "crypt/CRC.h"
#include "time/time.hpp"

namespace Network {
    const static uint32_t MAX_MESSAGE_LENGTH = 32 * 1024;//网络层限制最大包长

    struct message_head {
        message_head() : length_(0) {}

        uint32_t length_;
    };

    inline message_head CreateMessageHeader(const char *data, uint32_t length) {
        message_head head;
        head.length_ = length;
        return head;
    }

};

