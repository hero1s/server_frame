
#include "redisclient/redis_client_wrap.h"

using namespace std;
using namespace svrlib;

namespace {

};

CRedisWrap::CRedisWrap()
        : m_timer(this) {
    m_asyncConnecting = false;
}

CRedisWrap::~CRedisWrap() {
}

void CRedisWrap::OnTimer() {
    CApplication::Instance().schedule(&m_timer, 5000);

    try
    {
        m_asyncClient->command("PING", {}, [this](const redisclient::RedisValue &v)
        {
            if (v.isError() || v.toString() != "PONG")
            {
                LOG_DEBUG("Async PING error rep:{}", v.toString());
                Reconnect(true, false);
            }
        });
    }
    catch (const asio::system_error &e)
    {
        LOG_ERROR("async redis throw error:{}", e.what());
        Reconnect(false, true);
        return;
    }
    try
    {
        auto result = m_syncClient->command("PING", {});
        if (result.isError() || result.toString() != "PONG")
        {
            LOG_DEBUG("Sync PING error rep:{}", result.toString());
            Reconnect(true, false);
        }
    }
    catch (const asio::system_error &e)
    {
        LOG_ERROR("sync redis throw error:{}", e.what());
        Reconnect(true, false);
        return;
    }

}

bool CRedisWrap::Init(asio::io_context &context, const stRedisConf & conf) {
    m_conf = conf;
    CApplication::Instance().schedule(&m_timer, 20000);

    //同步客户端
    m_syncClient = std::make_shared<redisclient::RedisSyncClient>(context);
    m_syncClient->setConnectTimeout(std::chrono::seconds(5)).setCommandTimeout(std::chrono::seconds(5));

    //异步客户端
    m_asyncClient = std::make_shared<redisclient::RedisAsyncClient>(context);

    m_syncClient->installErrorHandler(std::bind(&CRedisWrap::HandSyncError, this, std::placeholders::_1));
    m_asyncClient->installErrorHandler(std::bind(&CRedisWrap::HandAsyncError, this, std::placeholders::_1));

    return Reconnect(true, true);
}

void CRedisWrap::ShutDown() {
    m_timer.cancel();
}

bool CRedisWrap::Reconnect(bool bSync, bool bAsync) {
    asio::ip::address address = asio::ip::address::from_string(m_conf.redisHost);
    const unsigned short port = m_conf.redisPort;
    asio::ip::tcp::endpoint endpoint(address, port);
    asio::error_code ec;
    redisclient::RedisValue result;
    //同步客户端
    if (bSync)
    {
        try
        {
            //同步客户端
            m_syncClient->disconnect();
            m_syncClient->connect(endpoint, ec);
            if (ec)
            {
                LOG_ERROR("Sync Can't connect to redis: {},{}", ec.message(), endpoint.address().to_string());
                return false;
            }
            else
            {
                result = m_syncClient->command("AUTH", {m_conf.redisPasswd});
                if (result.isOk() && result.toString() == "OK")
                {
                    LOG_DEBUG("Sync connect redis {} ok and auth is ok", endpoint.address().to_string());
                    TestConnect(true, false);
                }
                else
                {
                    LOG_ERROR("Sync redis auth is error:{}", result.toString());
                    return false;
                }
            }
        }
        catch (const asio::system_error &e)
        {
            LOG_ERROR("sync redis connect throw error:{},{}", e.what(), endpoint.address().to_string());
            return false;
        }
    }
    //异步客户端
    if (bAsync)
    {
        try
        {
            //异步客户端
            if (m_asyncConnecting)
            {
                LOG_ERROR("asyncConnecting...");
                return false;
            }
            m_asyncConnecting = true;
            m_asyncClient->disconnect();
            m_asyncClient->connect(endpoint, [this](asio::error_code ec)
            {
                if (!ec)
                {
                    m_asyncClient->command("AUTH", {m_conf.redisPasswd}, [&](const redisclient::RedisValue &v)
                    {
                        m_asyncConnecting = false;
                        if (v.isOk() && v.toString() == "OK")
                        {
                            LOG_DEBUG("async redis client connect ok and auth ok");
                            TestConnect(false, true);
                        }
                        else
                        {
                            LOG_ERROR("async redis client connect error:{}", v.toString());
                        }
                    });
                }
                else
                {
                    m_asyncConnecting = false;
                    LOG_ERROR("async Can't connect to redis: {}", ec.message());
                }
            });
        }
        catch (const asio::system_error &e)
        {
            LOG_ERROR("async redis connect throw error:{},{}", e.what(), endpoint.address().to_string());
            m_asyncConnecting = false;
            return false;
        }
    }
    return true;
}

void CRedisWrap::TestConnect(bool bSync, bool bAsync) {
    if (bSync)
    {
        auto result = SafeSyncCommond("SET", {"key", "value"});
        if (result.isError())
        {
            LOG_ERROR("SET error: {}", result.toString());
            return;
        }
        result = SafeSyncCommond("GET", {"key"});
        if (result.isOk())
        {
            LOG_DEBUG("GET: {}", result.toString());
        }
        else
        {
            LOG_ERROR("GET error: {}", result.toString());
            return;
        }
    }
    if (bAsync)
    {
        SafeAsyncCommond("SET", {"key", "value"},
                         [this](const redisclient::RedisValue &v)
                         {
                             LOG_DEBUG("async SET: {}", v.toString());
                             SafeAsyncCommond("GET", {"key"},
                                              [this](const redisclient::RedisValue &v)
                                              {
                                                  LOG_DEBUG("async GET: {}",
                                                            v.toString());
                                              });
                         });
    }
}

void CRedisWrap::HandSyncError(const std::string &err) {
    LOG_ERROR("redis sync error:{}", err);
    //m_syncClient->disconnect();
    //Reconnect(true, false);
}

void CRedisWrap::HandAsyncError(const std::string &err) {
    LOG_ERROR("redis async error:{}", err);
    //m_asyncClient->disconnect();
    //Reconnect(false, true);
}

redisclient::RedisValue CRedisWrap::SafeSyncCommond(const std::string &cmd, std::deque<redisclient::RedisBuffer> args) {
    try
    {
        return m_syncClient->command(cmd, args);
    }
    catch (const asio::system_error &e)
    {
        LOG_ERROR("sync redis:{}, throw error:{}",cmd, e.what());
        Reconnect(true, false);
    }
    redisclient::RedisValue::ErrorTag tag;
    return redisclient::RedisValue({}, tag);
}

void CRedisWrap::SafeAsyncCommond(const std::string &cmd, std::deque<redisclient::RedisBuffer> args,
                                 std::function<void(redisclient::RedisValue)> handler) {
    try
    {
        m_asyncClient->command(cmd, args, handler);
    }
    catch (const asio::system_error &e)
    {
        LOG_ERROR("async redis:{}, throw error:{}",cmd, e.what());
        //Reconnect(false, true);
    }
}




