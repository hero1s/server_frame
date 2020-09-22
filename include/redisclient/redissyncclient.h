/*
 * Copyright (C) Alex Nekipelov (alex@nekipelov.net)
 * License: MIT
 */

#ifndef REDISSYNCCLIENT_REDISCLIENT_H
#define REDISSYNCCLIENT_REDISCLIENT_H

#include <asio/io_service.hpp>
#include <asio/detail/noncopyable.hpp>

#include <string>
#include <list>
#include <functional>

#include "redisclient/impl/redisclientimpl.h"
#include "redisbuffer.h"
#include "redisvalue.h"
#include "config.h"

namespace redisclient {

class RedisClientImpl;
class Pipeline;

class RedisSyncClient : asio::detail::noncopyable {
public:
    typedef RedisClientImpl::State State;

    REDIS_CLIENT_DECL RedisSyncClient(asio::io_service &ioService);
    REDIS_CLIENT_DECL RedisSyncClient(RedisSyncClient &&other);
    REDIS_CLIENT_DECL ~RedisSyncClient();

    // Connect to redis server
    REDIS_CLIENT_DECL void connect(
            const asio::ip::tcp::endpoint &endpoint,
            asio::error_code &ec);

    // Connect to redis server
    REDIS_CLIENT_DECL void connect(
            const asio::ip::tcp::endpoint &endpoint);

#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
    REDIS_CLIENT_DECL void connect(
            const asio::local::stream_protocol::endpoint &endpoint,
            asio::error_code &ec);

    REDIS_CLIENT_DECL void connect(
            const asio::local::stream_protocol::endpoint &endpoint);
#endif

    // Return true if is connected to redis.
    REDIS_CLIENT_DECL bool isConnected() const;

    // disconnect from redis
    REDIS_CLIENT_DECL void disconnect();

    // Set custom error handler.
    REDIS_CLIENT_DECL void installErrorHandler(
        std::function<void(const std::string &)> handler);

    // Execute command on Redis server with the list of arguments.
    REDIS_CLIENT_DECL RedisValue command(
            std::string cmd, std::deque<RedisBuffer> args);

    // Execute command on Redis server with the list of arguments.
    REDIS_CLIENT_DECL RedisValue command(
            std::string cmd, std::deque<RedisBuffer> args,
            asio::error_code &ec);

    // Create pipeline (see Pipeline)
    REDIS_CLIENT_DECL Pipeline pipelined();

    REDIS_CLIENT_DECL RedisValue pipelined(
            std::deque<std::deque<RedisBuffer>> commands,
            asio::error_code &ec);

    REDIS_CLIENT_DECL RedisValue pipelined(
            std::deque<std::deque<RedisBuffer>> commands);

    // Return connection state. See RedisClientImpl::State.
    REDIS_CLIENT_DECL State state() const;

    REDIS_CLIENT_DECL RedisSyncClient &setConnectTimeout(
            const std::chrono::seconds &timeout);
    REDIS_CLIENT_DECL RedisSyncClient &setCommandTimeout(
            const std::chrono::seconds &timeout);

    REDIS_CLIENT_DECL RedisSyncClient &setTcpNoDelay(bool enable);
    REDIS_CLIENT_DECL RedisSyncClient &setTcpKeepAlive(bool enable);

protected:
    REDIS_CLIENT_DECL bool stateValid() const;

private:
    std::shared_ptr<RedisClientImpl> pimpl;
    std::chrono::seconds connectTimeout;
    std::chrono::seconds commandTimeout;
    bool tcpNoDelay;
    bool tcpKeepAlive;
};

}

#ifdef REDIS_CLIENT_HEADER_ONLY
#include "redisclient/impl/redissyncclient.cpp"
#endif

#endif // REDISSYNCCLIENT_REDISCLIENT_H
