
#pragma once

#include "svrlib.h"
#include "config/config.h"
#include <string>
#include "redisclient/redisasyncclient.h"
#include "redisclient/redissyncclient.h"
#include "asio.hpp"

using namespace std;
using namespace svrlib;

using reply_callback_t = std::function<void(redisclient::RedisValue v)>;

class CRedisWrap {
public:
    CRedisWrap();

    virtual ~CRedisWrap();

    virtual void OnTimer();

    virtual bool Init(asio::io_context &context, const stRedisConf & conf);

    virtual void ShutDown();

protected:
    virtual bool Reconnect(bool bSync, bool bAsync);

    virtual void TestConnect(bool bSync, bool bAsync);

    virtual void HandSyncError(const std::string &err);

    virtual void HandAsyncError(const std::string &err);

    virtual redisclient::RedisValue SafeSyncCommond(const std::string &cmd, std::deque<redisclient::RedisBuffer> args);

    virtual void SafeAsyncCommond(const std::string &cmd, std::deque<redisclient::RedisBuffer> args,
                          std::function<void(redisclient::RedisValue)> handler = [](redisclient::RedisValue){});

protected:
    MemberTimerEvent<CRedisWrap, &CRedisWrap::OnTimer> m_timer;
    stRedisConf m_conf;

    std::shared_ptr<redisclient::RedisSyncClient> m_syncClient = nullptr;
    std::shared_ptr<redisclient::RedisAsyncClient> m_asyncClient = nullptr;

    bool m_asyncConnecting = false;
};
/*
    ע�⣺connect,commond,pipeline���������쳣,pipe���߻����쳣����ʱ��ò���
*/


























































































































































