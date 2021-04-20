
#pragma once

#include "tcp_callbacks.h"
#include "tcp_conn.h"
#include <asio.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/steady_timer.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

using asio::steady_timer;
using asio::ip::tcp;

namespace Network {
class TCPServer {
public:
    TCPServer(asio::io_service& service_, const std::string& bind_ip, uint16_t port, const std::string& name,
        SocketParserType parserType = SocketParserType_TCP);

    ~TCPServer();

    bool Start();

    void Stop();

    void SetConnectionCallback(const ConnCallback& cb) { conn_fn_ = cb; }

    void SetMessageCallback(const MessageCallback& cb) { msg_fn_ = cb; }

    const std::string Address() const
    {
        return acceptor_.local_endpoint().address().to_string();
    }

protected:
    void StopInLoop();

    void RemoveConn(const TCPConnPtr& conn);

    void AsyncAccept();

    void HandleNewConn(tcp::socket&& socket);

    void HeartBeatTimer();

private:
    asio::io_service& io_service_; // the listening loop
    tcp::acceptor acceptor_;
    tcp::socket accept_socket_; // default accept this socket

    bool closed_;
    const std::string name_;
    ConnCallback conn_fn_;
    MessageCallback msg_fn_;

    // always in the listening loop thread
    uint64_t next_conn_id_;
    std::unordered_map<std::string, TCPConnPtr> conns_;
    steady_timer heartbeat_timer_;
    uint32_t disconnect_time_;
    SocketParserType parserType_;
};
};
