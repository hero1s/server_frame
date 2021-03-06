
#pragma once

#include "message_head.h"
#include <functional>
#include <memory>

namespace Network {
class TCPConn;
typedef std::shared_ptr<TCPConn> TCPConnPtr;
typedef std::shared_ptr<MsgDecode> MsgDecodePtr;

typedef std::function<void(const TCPConnPtr&)> ConnCallback;
typedef std::function<void(const TCPConnPtr&)> CloseCallback;

typedef std::function<void(const TCPConnPtr&)> WriteCompleteCallback;

typedef std::function<void(const TCPConnPtr&, uint32_t)> HighWaterMarkCallback;

typedef std::function<void(const TCPConnPtr&, const char* pData, uint32_t length)> MessageCallback;

void DefaultConnectionCallback(const TCPConnPtr& connPtr);

void DefaultMessageCallback(const TCPConnPtr& connPtr, const char* pData, uint32_t length);

void DefaultHighWaterMarkCallback(const TCPConnPtr& connPtr, uint32_t len);

};
