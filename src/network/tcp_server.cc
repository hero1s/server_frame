
#include "network/tcp_server.h"
#include "network/tcp_conn.h"
#include "utility/comm_macro.h"
#include <assert.h>

namespace Network {
using asio::ip::address;

TCPServer::TCPServer(asio::io_service& service_, const std::string& bind_ip, uint16_t port, const std::string& name,
    SocketParserType parserType)
    : io_service_(service_)
    , acceptor_(io_service_, tcp::endpoint(address::from_string(bind_ip), port))
    , accept_socket_(io_service_)
    , closed_(false)
    , name_(name)
    , conn_fn_(DefaultConnectionCallback)
    , msg_fn_(DefaultMessageCallback)
    , next_conn_id_(0)
    , heartbeat_timer_(io_service_)
    , disconnect_time_(0)
    , parserType_(parserType)
{
    acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
}

TCPServer::~TCPServer()
{
    assert(conns_.empty());
    assert(!acceptor_.is_open());
}

bool TCPServer::Start()
{
    AsyncAccept();
    HeartBeatTimer();
    return true;
}

void TCPServer::Stop()
{
    StopInLoop();
}

void TCPServer::StopInLoop()
{
    asio::error_code ec;
    acceptor_.cancel();
    acceptor_.close(ec);

    for (auto& c : conns_) {
        c.second->Close();
    }

    closed_ = true;
}

void TCPServer::RemoveConn(const TCPConnPtr& conn)
{
    // Remove the connection in the listening EventLoop
    this->conns_.erase(conn->GetName());
}

void TCPServer::AsyncAccept()
{
    tcp::socket* socket = nullptr;
    socket = &accept_socket_;
    acceptor_.async_accept(*socket, [this, socket](asio::error_code ec) {
        if (!ec) {
            asio::error_code error;
            socket->non_blocking(true, error);
            this->HandleNewConn(std::move(*socket));
        } else {
            // error notice
            LOG_ERROR("accept error:{}", ec.message());
        }
        if (!closed_) {
            this->AsyncAccept();
        }
    });
}

void TCPServer::HandleNewConn(tcp::socket&& socket)
{
    std::string cn = name_ + "-conn" + "#" + std::to_string(next_conn_id_++); // TODO use string buffer
    TCPConnPtr conn = std::make_shared<TCPConn>(io_service_, std::move(socket), cn, parserType_);
    assert(conn->GetType() == TCPConn::kIncoming);
    conn->SetMessageCallback(msg_fn_);
    conn->SetConnCallback(conn_fn_);
    conn->SetCloseCallback(std::bind(&TCPServer::RemoveConn, this, std::placeholders::_1));
    conn->OnAttachedToLoop();
    conns_[cn] = conn;
}

void TCPServer::HeartBeatTimer()
{
    time_t curTime = TCPConn::Now();
    for (auto& conn : conns_) {
        conn.second->TimeOut(curTime);
    }

    heartbeat_timer_.expires_from_now(std::chrono::seconds(2));
    heartbeat_timer_.async_wait(std::bind(&TCPServer::HeartBeatTimer, this));
}

};