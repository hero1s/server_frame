
#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include <atomic>
#include <mutex>
#include <asio/steady_timer.hpp>
#include <asio/ip/tcp.hpp>
#include <asio.hpp>
#include "tcp_callbacks.h"

using asio::ip::tcp;
namespace Network {
    class TCPClient {
    public:
        TCPClient(asio::io_service &service_, const std::string &remote_addr, uint16_t port, const std::string &name);

        ~TCPClient();

        void Connect();

        void Disconnect();

        void SetConnCallback(const ConnCallback &cb);

        void SetMessageCallback(const MessageCallback &cb) { msg_fn_ = cb; }

        bool GetAutoReconnect() const { return auto_reconnect_; }

        void SetAutoReconnect(bool v) { auto_reconnect_.store(v); }

        uint64_t GetReconnectInterval() const { return reconnect_interval_; }

        void SetReconnectInterval(uint64_t timeout) { reconnect_interval_ = timeout; }

        TCPConnPtr GetTCPConn() const;

        const std::string &RemoteAddress() const { return remote_addr_; }

        uint16_t RemotePort() const { return remote_port_; }

        const std::string &GetName() const { return name_; }

        void SetUID(uint32_t uid) { uid_ = uid; }

        uint32_t GetUID() { return uid_; }

    private:
        void DisconnectInLoop();

        void HandleConn(tcp::socket &&socket);

        void OnRemoveConn(const TCPConnPtr &conn);

        void Reconnect();

    private:
        asio::io_service &io_service_;

        tcp::socket socket_;

        std::string remote_addr_;
        uint16_t remote_port_;

        std::string name_;
        uint32_t uid_;

        asio::system_timer reconnect_timer_;
        std::atomic<bool> auto_reconnect_; // The flag whether it reconnects automatically, Default : true
        uint64_t reconnect_interval_; // Default : 3 seconds
        int reconnecting_times_; // Default : 3 seconds

        TCPConnPtr conn_;

        ConnCallback conn_fn_;
        MessageCallback msg_fn_;
    };
};

