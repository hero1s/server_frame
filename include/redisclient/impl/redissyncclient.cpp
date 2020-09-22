/*
 * Copyright (C) Alex Nekipelov (alex@nekipelov.net)
 * License: MIT
 */

#ifndef REDISCLIENT_REDISSYNCCLIENT_CPP
#define REDISCLIENT_REDISSYNCCLIENT_CPP

#include <memory>
#include <functional>

#include "redisclient/redissyncclient.h"
#include "redisclient/pipeline.h"
#include "redisclient/impl/throwerror.h"

namespace redisclient {

RedisSyncClient::RedisSyncClient(asio::io_service &ioService)
    : pimpl(std::make_shared<RedisClientImpl>(ioService)),
    connectTimeout(std::chrono::hours(365*24)),
    commandTimeout(std::chrono::hours(365*24)),
    tcpNoDelay(true), tcpKeepAlive(false)
{
    pimpl->errorHandler = std::bind(&RedisClientImpl::defaulErrorHandler, std::placeholders::_1);
}

RedisSyncClient::RedisSyncClient(RedisSyncClient &&other)
    : pimpl(std::move(other.pimpl)),
    connectTimeout(std::move(other.connectTimeout)),
    commandTimeout(std::move(other.commandTimeout)),
    tcpNoDelay(std::move(other.tcpNoDelay)),
    tcpKeepAlive(std::move(other.tcpKeepAlive))
{
}


RedisSyncClient::~RedisSyncClient()
{
    if (pimpl)
        pimpl->close();
}

void RedisSyncClient::connect(const asio::ip::tcp::endpoint &endpoint)
{
    asio::error_code ec;

    connect(endpoint, ec);
    detail::throwIfError(ec);
}

void RedisSyncClient::connect(const asio::ip::tcp::endpoint &endpoint,
    asio::error_code &ec)
{
    pimpl->socket.open(endpoint.protocol(), ec);

    if (!ec && tcpNoDelay)
        pimpl->socket.set_option(asio::ip::tcp::no_delay(true), ec);

    // TODO keep alive option

    // boost asio does not support `connect` with timeout
    int socket = pimpl->socket.native_handle();
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(endpoint.port());
    addr.sin_addr.s_addr = inet_addr(endpoint.address().to_string().c_str());

    // Set non-blocking
    int arg = 0;
    if ((arg = fcntl(socket, F_GETFL, NULL)) < 0)
    {
        ec = asio::error_code(errno,
                asio::error::get_system_category());
        return;
    }

    arg |= O_NONBLOCK;

    if (fcntl(socket, F_SETFL, arg) < 0)
    {
        ec = asio::error_code(errno,
                asio::error::get_system_category());
        return;
    }

    // connecting
    int result = ::connect(socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    if (result < 0)
    {
        if (errno == EINPROGRESS)
        {
            for(;;)
            {
                //selecting
                pollfd pfd;
                pfd.fd = socket;
                pfd.events = POLLOUT;

                result = ::poll(&pfd, 1, std::chrono::milliseconds(connectTimeout).count());

                if (result < 0)
                {
                    if (errno == EINTR)
                    {
                        // try again
                        continue;
                    }
                    else
                    {
                        ec = asio::error_code(errno,
                                asio::error::get_system_category());
                        return;
                    }
                }
                else if (result > 0)
                {
                    // check for error
                    int valopt;
                    socklen_t optlen = sizeof(valopt);


                    if (getsockopt(socket, SOL_SOCKET, SO_ERROR,
                                reinterpret_cast<void *>(&valopt), &optlen ) < 0)
                    {
                        ec = asio::error_code(errno,
                                asio::error::get_system_category());
                        return;
                    }

                    if (valopt)
                    {
                        ec = asio::error_code(valopt,
                                asio::error::get_system_category());
                        return;
                    }

                    break;
                }
                else
                {
                    // timeout
                    ec = asio::error_code(ETIMEDOUT,
                            asio::error::get_system_category());
                    return;
                }
            }
        }
        else
        {
            ec = asio::error_code(errno,
                    asio::error::get_system_category());
            return;
        }
    }

    if ((arg = fcntl(socket, F_GETFL, NULL)) < 0)
    {
        ec = asio::error_code(errno,
                asio::error::get_system_category());
        return;
    }

    arg &= (~O_NONBLOCK); 

    if (fcntl(socket, F_SETFL, arg) < 0)
    {
        ec = asio::error_code(errno,
                asio::error::get_system_category());
    }

    if (!ec)
        pimpl->state = State::Connected;
}

#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS

void RedisSyncClient::connect(const asio::local::stream_protocol::endpoint &endpoint)
{
    asio::error_code ec;

    connect(endpoint, ec);
    detail::throwIfError(ec);
}

void RedisSyncClient::connect(const asio::local::stream_protocol::endpoint &endpoint,
        asio::error_code &ec)
{
    pimpl->socket.open(endpoint.protocol(), ec);

    if (!ec)
        pimpl->socket.connect(endpoint, ec);

    if (!ec)
        pimpl->state = State::Connected;
}

#endif

bool RedisSyncClient::isConnected() const
{
    return pimpl->getState() == State::Connected ||
            pimpl->getState() == State::Subscribed;
}

void RedisSyncClient::disconnect()
{
    pimpl->close();
}

void RedisSyncClient::installErrorHandler(
        std::function<void(const std::string &)> handler)
{
    pimpl->errorHandler = std::move(handler);
}

RedisValue RedisSyncClient::command(std::string cmd, std::deque<RedisBuffer> args)
{
    asio::error_code ec;
    RedisValue result = command(std::move(cmd), std::move(args), ec);

    detail::throwIfError(ec);
    return result;
}

RedisValue RedisSyncClient::command(std::string cmd, std::deque<RedisBuffer> args,
            asio::error_code &ec)
{
    if(stateValid())
    {
        args.push_front(std::move(cmd));

        return pimpl->doSyncCommand(args, commandTimeout, ec);
    }
    else
    {
        return RedisValue();
    }
}

Pipeline RedisSyncClient::pipelined()
{
    Pipeline pipe(*this);
    return pipe;
}

RedisValue RedisSyncClient::pipelined(std::deque<std::deque<RedisBuffer>> commands)
{
    asio::error_code ec;
    RedisValue result = pipelined(std::move(commands), ec);

    detail::throwIfError(ec);
    return result;
}

RedisValue RedisSyncClient::pipelined(std::deque<std::deque<RedisBuffer>> commands,
        asio::error_code &ec)
{
    if(stateValid())
    {
        return pimpl->doSyncCommand(commands, commandTimeout, ec);
    }
    else
    {
        return RedisValue();
    }
}

RedisSyncClient::State RedisSyncClient::state() const
{
    return pimpl->getState();
}

bool RedisSyncClient::stateValid() const
{
    assert( state() == State::Connected );

    if( state() != State::Connected )
    {
        std::stringstream ss;

        ss << "RedisClient::command called with invalid state "
           << to_string(state());

        pimpl->errorHandler(ss.str());
        return false;
    }

    return true;
}

RedisSyncClient &RedisSyncClient::setConnectTimeout(
        const std::chrono::seconds &timeout)
{
    connectTimeout = timeout;
    return *this;
}


RedisSyncClient &RedisSyncClient::setCommandTimeout(
        const std::chrono::seconds &timeout)
{
    commandTimeout = timeout;
    return *this;
}

RedisSyncClient &RedisSyncClient::setTcpNoDelay(bool enable)
{
    tcpNoDelay = enable;
    return *this;
}

RedisSyncClient &RedisSyncClient::setTcpKeepAlive(bool enable)
{
    tcpKeepAlive = enable;
    return *this;
}

}

#endif // REDISCLIENT_REDISSYNCCLIENT_CPP
