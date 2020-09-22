
#pragma once

#include <memory>
#include <functional>

namespace Network {
    class TCPConn;

    typedef std::shared_ptr<TCPConn> TCPConnPtr;

    typedef std::function<void(const TCPConnPtr &)> ConnCallback;

    typedef std::function<void(const TCPConnPtr &)> CloseCallback;
    typedef std::function<void(const TCPConnPtr &)> WriteCompleteCallback;

    typedef std::function<void(const TCPConnPtr &, size_t)> HighWaterMarkCallback;

    typedef std::function<void(const TCPConnPtr &, uint8_t* pData,uint32_t length)> MessageCallback;

    void DefaultConnectionCallback(const TCPConnPtr & connPtr);

    void DefaultMessageCallback(const TCPConnPtr & connPtr, uint8_t* pData,uint32_t length);

    void DefaultHighWaterMarkCallback(const TCPConnPtr & connPtr, size_t len);

};
