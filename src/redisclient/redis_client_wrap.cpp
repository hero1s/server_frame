
#include "redisclient/redis_client_wrap.h"

using namespace std;
using namespace svrlib;

namespace {

};

CRedisWrap::CRedisWrap() {
    m_asyncConnecting = false;
}

CRedisWrap::~CRedisWrap() {
    if (m_pTimer != nullptr) {
        m_pTimer->cancel();
    }
}

void CRedisWrap::OnTimer(const std::error_code &err) {
    if (!err) {
        m_pTimer->expires_from_now(std::chrono::seconds(5));
        m_pTimer->async_wait(std::bind(&CRedisWrap::OnTimer, this, std::placeholders::_1));
    } else {
        LOG_ERROR("asio timer is error or cancel,shutdown");
    }

    try {
        m_asyncClient->command("PING", {}, [this](const redisclient::RedisValue &v) {
            if (v.isError() || v.toString() != "PONG") {
                LOG_DEBUG("Async PING error rep:{}", v.toString());
                Reconnect(true, false);
            }
        });
    }
    catch (const asio::system_error &e) {
        LOG_ERROR("async redis throw error:{}", e.what());
        Reconnect(false, true);
        return;
    }
    try {
        auto result = m_syncClient->command("PING", {});
        if (result.isError() || result.toString() != "PONG") {
            LOG_DEBUG("Sync PING error rep:{}", result.toString());
            Reconnect(true, false);
        }
    }
    catch (const asio::system_error &e) {
        LOG_ERROR("sync redis throw error:{}", e.what());
        Reconnect(true, false);
        return;
    }

}

bool CRedisWrap::Init(asio::io_context &context, const stRedisConf &conf) {
    m_conf = conf;
    m_pTimer = make_shared<asio::system_timer>(context);
    m_pTimer->expires_from_now(std::chrono::seconds(20));
    m_pTimer->async_wait(std::bind(&CRedisWrap::OnTimer, this, std::placeholders::_1));

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
    if (m_pTimer != nullptr) {
        m_pTimer->cancel();
    }
}

bool CRedisWrap::Reconnect(bool bSync, bool bAsync) {
    asio::ip::address address = asio::ip::address::from_string(m_conf.redisHost);
    const unsigned short port = m_conf.redisPort;
    asio::ip::tcp::endpoint endpoint(address, port);
    asio::error_code ec;
    redisclient::RedisValue result;
    //同步客户端
    if (bSync) {
        try {
            //同步客户端
            m_syncClient->disconnect();
            m_syncClient->connect(endpoint, ec);
            if (ec) {
                LOG_ERROR("Sync Can't connect to redis: {},{}", ec.message(), endpoint.address().to_string());
                return false;
            } else {
                result = m_syncClient->command("AUTH", {m_conf.redisPasswd});
                if (result.isOk() && result.toString() == "OK") {
                    LOG_DEBUG("Sync connect redis {} ok and auth is ok", endpoint.address().to_string());
                    TestConnect(true, false);
                } else {
                    LOG_ERROR("Sync redis auth is error:{}", result.toString());
                    return false;
                }
            }
        }
        catch (const asio::system_error &e) {
            LOG_ERROR("sync redis connect throw error:{},{}", e.what(), endpoint.address().to_string());
            return false;
        }
    }
    //异步客户端
    if (bAsync) {
        try {
            //异步客户端
            if (m_asyncConnecting) {
                LOG_ERROR("asyncConnecting...");
                return false;
            }
            m_asyncConnecting = true;
            m_asyncClient->disconnect();
            m_asyncClient->connect(endpoint, [this](asio::error_code ec) {
                if (!ec) {
                    m_asyncClient->command("AUTH", {m_conf.redisPasswd}, [&](const redisclient::RedisValue &v) {
                        m_asyncConnecting = false;
                        if (v.isOk() && v.toString() == "OK") {
                            LOG_DEBUG("async redis client connect ok and auth ok");
                            TestConnect(false, true);
                        } else {
                            LOG_ERROR("async redis client connect error:{}", v.toString());
                        }
                    });
                } else {
                    m_asyncConnecting = false;
                    LOG_ERROR("async Can't connect to redis: {}", ec.message());
                }
            });
        }
        catch (const asio::system_error &e) {
            LOG_ERROR("async redis connect throw error:{},{}", e.what(), endpoint.address().to_string());
            m_asyncConnecting = false;
            return false;
        }
    }
    return true;
}

void CRedisWrap::TestConnect(bool bSync, bool bAsync) {
    if (bSync) {
        auto result = SafeSyncCommond("SET", {"key", "value"});
        if (result.isError()) {
            LOG_ERROR("SET error: {}", result.toString());
            return;
        }
        result = SafeSyncCommond("GET", {"key"});
        if (result.isOk()) {
            LOG_DEBUG("GET: {}", result.toString());
        } else {
            LOG_ERROR("GET error: {}", result.toString());
            return;
        }
    }
    if (bAsync) {
        SafeAsyncCommond("SET", {"key", "value"},
                         [this](const redisclient::RedisValue &v) {
                             LOG_DEBUG("async SET: {}", v.toString());
                             SafeAsyncCommond("GET", {"key"},
                                              [this](const redisclient::RedisValue &v) {
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
    try {
        return m_syncClient->command(cmd, args);
    }
    catch (const asio::system_error &e) {
        LOG_ERROR("sync redis:{}, throw error:{}", cmd, e.what());
        Reconnect(true, false);
    }
    redisclient::RedisValue::ErrorTag tag;
    return redisclient::RedisValue({}, tag);
}

void CRedisWrap::SafeAsyncCommond(const std::string &cmd, std::deque<redisclient::RedisBuffer> args,
                                  std::function<void(redisclient::RedisValue)> handler) {
    try {
        m_asyncClient->command(cmd, args, handler);
    }
    catch (const asio::system_error &e) {
        LOG_ERROR("async redis:{}, throw error:{}", cmd, e.what());
        //Reconnect(false, true);
    }
}


//常用命令
redisclient::RedisValue CRedisWrap::append(const std::string &key, const std::string &value) {
    return SafeSyncCommond("APPEND", {key, value});
}

redisclient::RedisValue CRedisWrap::auth(const std::string &password) {
    return SafeSyncCommond("AUTH", {password});
}

redisclient::RedisValue CRedisWrap::bgrewriteaof() {
    return SafeSyncCommond("BGREWRITEAOF", {});
}

redisclient::RedisValue CRedisWrap::bgsave() {
    return SafeSyncCommond("BGSAVE", {});
}

redisclient::RedisValue CRedisWrap::bitcount(const std::string &key) {
    return SafeSyncCommond("BITCOUNT", {key});
}

redisclient::RedisValue CRedisWrap::bitcount(const std::string &key, int start, int end) {
    return SafeSyncCommond("BITCOUNT", {key, start, end});
}

//hash
redisclient::RedisValue CRedisWrap::hdel(const std::string &key, const std::vector<std::string> &fields) {
    std::deque<redisclient::RedisBuffer> args;
    args.emplace_back(key);
    for (const auto &obj:fields) {
        args.emplace_back(obj);
    }
    return SafeSyncCommond("HDEL", args);
}

redisclient::RedisValue CRedisWrap::hexists(const std::string &key, const std::string &field) {
    return SafeSyncCommond("HEXISTS", {key, field});
}

redisclient::RedisValue CRedisWrap::hget(const std::string &key, const std::string &field) {
    return SafeSyncCommond("HGET", {key, field});
}

redisclient::RedisValue CRedisWrap::hgetall(const std::string &key) {
    return SafeSyncCommond("HGETALL", {key});
}

redisclient::RedisValue CRedisWrap::hincrby(const std::string &key, const std::string &field, int incr) {
    return SafeSyncCommond("HINCRBY", {key, field, incr});
}

redisclient::RedisValue CRedisWrap::hincrbyfloat(const std::string &key, const std::string &field, float incr) {
    return SafeSyncCommond("HINCRBYFLOAT", {key, field, incr});
}

redisclient::RedisValue CRedisWrap::hkeys(const std::string &key) {
    return SafeSyncCommond("HKEYS", {key});
}

redisclient::RedisValue CRedisWrap::hlen(const std::string &key) {
    return SafeSyncCommond("HLEN", {key});
}

redisclient::RedisValue CRedisWrap::hmget(const std::string &key, const std::vector<std::string> &fields) {
    std::deque<redisclient::RedisBuffer> args;
    args.emplace_back(key);
    for (const auto &obj:fields) {
        args.emplace_back(obj);
    }
    return SafeSyncCommond("HMGET", args);
}

redisclient::RedisValue
CRedisWrap::hmset(const std::string &key, const std::vector<std::pair<std::string, std::string>> &field_vals) {
    std::deque<redisclient::RedisBuffer> args;
    args.emplace_back(key);
    for (const auto &obj:field_vals) {
        args.emplace_back(obj.first);
        args.emplace_back(obj.second);
    }
    return SafeSyncCommond("HMSET", args);
}

redisclient::RedisValue CRedisWrap::hset(const std::string &key, const std::string &field, const std::string &value) {
    return SafeSyncCommond("HSET", {key, field, value});
}

redisclient::RedisValue CRedisWrap::hscan(const std::string &key, const std::string &field, const std::string &value) {
    return SafeSyncCommond("HSCAN", {key, field, value});
}

redisclient::RedisValue CRedisWrap::hsetnx(const std::string &key, const std::string &field, const std::string &value) {
    return SafeSyncCommond("HSETNX", {key, field, value});
}

redisclient::RedisValue CRedisWrap::hstrlen(const std::string &key, const std::string &field) {
    return SafeSyncCommond("HSTRLEN", {key, field});
}

redisclient::RedisValue CRedisWrap::hvals(const std::string &key) {
    return SafeSyncCommond("HVALS", {key});
}

//list
redisclient::RedisValue CRedisWrap::blpop(const std::vector<std::string> &keys, int timeout) {
    std::deque<redisclient::RedisBuffer> args;
    for (const auto &obj:keys) {
        args.emplace_back(obj);
    }
    args.emplace_back(timeout);

    return SafeSyncCommond("BLOP", args);
}

redisclient::RedisValue CRedisWrap::brpop(const std::vector<std::string> &keys, int timeout) {
    std::deque<redisclient::RedisBuffer> args;
    for (const auto &obj:keys) {
        args.emplace_back(obj);
    }
    args.emplace_back(timeout);

    return SafeSyncCommond("BRPOP", args);
}

redisclient::RedisValue CRedisWrap::brpoplpush(const std::string &src, std::string &dst, int timeout) {
    return SafeSyncCommond("BRPOPLPUSH", {src, dst, timeout});
}

redisclient::RedisValue CRedisWrap::lindex(const std::string &key, int index) {
    return SafeSyncCommond("LINDEX", {key, index});
}

redisclient::RedisValue
CRedisWrap::linsert(const std::string &key, const std::string &before_after, const std::string &pivot,
                    const std::string &value) {
    return SafeSyncCommond("LINSERT", {key, before_after, pivot, value});
}

redisclient::RedisValue CRedisWrap::llen(const std::string &key) {
    return SafeSyncCommond("LLEN", {key});
}

redisclient::RedisValue CRedisWrap::lpop(const std::string &key) {
    return SafeSyncCommond("LPOP", {key});
}

redisclient::RedisValue CRedisWrap::lpush(const std::string &key, const std::vector<std::string> &values) {
    std::deque<redisclient::RedisBuffer> args;
    args.emplace_back(key);
    for (const auto &obj:values) {
        args.emplace_back(obj);
    }
    return SafeSyncCommond("LPUSH", args);
}

redisclient::RedisValue CRedisWrap::lpushx(const std::string &key, const std::string &value) {
    return SafeSyncCommond("LPUSHX", {key, value});
}

redisclient::RedisValue CRedisWrap::lrange(const std::string &key, int start, int stop) {
    return SafeSyncCommond("LRANGE", {key, start, stop});
}

redisclient::RedisValue CRedisWrap::lrem(const std::string &key, int count, const std::string &value) {
    return SafeSyncCommond("LREM", {key, count, value});
}

redisclient::RedisValue CRedisWrap::lset(const std::string &key, int index, const std::string &value) {
    return SafeSyncCommond("LSET", {key, index, value});
}

redisclient::RedisValue CRedisWrap::ltrim(const std::string &key, int start, int stop) {
    return SafeSyncCommond("LTRIM", {key, start, stop});
}

redisclient::RedisValue CRedisWrap::rpop(const std::string &key) {
    return SafeSyncCommond("RPOP", {key});
}

redisclient::RedisValue CRedisWrap::rpoplpush(const std::string &src, std::string &dst) {
    return SafeSyncCommond("RPOPLPUSH", {src, dst});
}

redisclient::RedisValue CRedisWrap::rpush(const std::string &key, const std::vector<std::string> &values) {
    std::deque<redisclient::RedisBuffer> args;
    args.emplace_back(key);
    for (const auto &obj:values) {
        args.emplace_back(obj);
    }
    return SafeSyncCommond("RPUSH", args);
}

redisclient::RedisValue CRedisWrap::rpushx(const std::string &key, const std::string &value) {
    return SafeSyncCommond("RPUSHX", {key, value});
}

//set
redisclient::RedisValue CRedisWrap::sadd(const std::string &key, const std::vector<std::string> &members) {
    std::deque<redisclient::RedisBuffer> args;
    args.emplace_back(key);
    for (const auto &obj:members) {
        args.emplace_back(obj);
    }
    return SafeSyncCommond("SADD", args);
}

redisclient::RedisValue CRedisWrap::scard(const std::string &key) {
    return SafeSyncCommond("SCARD", {key});
}

redisclient::RedisValue CRedisWrap::sdiff(const std::vector<std::string> &keys) {
    std::deque<redisclient::RedisBuffer> args;
    for (const auto &obj:keys) {
        args.emplace_back(obj);
    }
    return SafeSyncCommond("SDIFF", args);
}

redisclient::RedisValue CRedisWrap::sdiffstore(const std::string &dst, const std::vector<std::string> &keys) {
    std::deque<redisclient::RedisBuffer> args;
    args.emplace_back(dst);
    for (const auto &obj:keys) {
        args.emplace_back(obj);
    }
    return SafeSyncCommond("SDIFFSTORE", args);
}

redisclient::RedisValue CRedisWrap::sinter(const std::vector<std::string> &keys) {
    std::deque<redisclient::RedisBuffer> args;
    for (const auto &obj:keys) {
        args.emplace_back(obj);
    }
    return SafeSyncCommond("SINTER", args);
}

redisclient::RedisValue CRedisWrap::sinterstore(const std::string &dst, const std::vector<std::string> &keys) {
    std::deque<redisclient::RedisBuffer> args;
    args.emplace_back(dst);
    for (const auto &obj:keys) {
        args.emplace_back(obj);
    }
    return SafeSyncCommond("SINTERSTORE", args);
}

redisclient::RedisValue CRedisWrap::sismember(const std::string &key, const std::string &member) {
    return SafeSyncCommond("SISMEMBER", {key, member});
}

redisclient::RedisValue CRedisWrap::smembers(const std::string &key) {
    return SafeSyncCommond("SMEMBERS", {key});
}

redisclient::RedisValue CRedisWrap::smove(const std::string &src, const std::string &dst, const std::string &member) {
    return SafeSyncCommond("SMOVE", {src, dst, member});
}

redisclient::RedisValue CRedisWrap::spop(const std::string &key, int count) {
    return SafeSyncCommond("SPOP", {key, count});
}

redisclient::RedisValue CRedisWrap::srandmember(const std::string &key, int count) {
    return SafeSyncCommond("SRANDMEMBER", {key, count});
}

redisclient::RedisValue CRedisWrap::srem(const std::string &key, const std::vector<std::string> &members) {
    std::deque<redisclient::RedisBuffer> args;
    args.emplace_back(key);
    for (const auto &obj:members) {
        args.emplace_back(obj);
    }
    return SafeSyncCommond("SREM", args);
}

//Redis& sscan;
redisclient::RedisValue CRedisWrap::sunion(const std::vector<std::string> &keys) {
    std::deque<redisclient::RedisBuffer> args;
    for (const auto &obj:keys) {
        args.emplace_back(obj);
    }
    return SafeSyncCommond("SUNION", args);
}

redisclient::RedisValue CRedisWrap::sunionstore(const std::string &dst, const std::vector<std::string> &keys) {
    std::deque<redisclient::RedisBuffer> args;
    args.emplace_back(dst);
    for (const auto &obj:keys) {
        args.emplace_back(obj);
    }
    return SafeSyncCommond("SUNIONSTORE", args);
}

//sorted sets
redisclient::RedisValue CRedisWrap::zadd(const std::string &key, const std::string &score, const std::string &member) {
    return SafeSyncCommond("ZADD", {key, score, member});
}

redisclient::RedisValue
CRedisWrap::zadd(const std::string &key, const std::vector<std::pair<std::string, std::string>> &score_members) {
    std::deque<redisclient::RedisBuffer> args;
    args.emplace_back(key);
    for (const auto &obj:score_members) {
        args.emplace_back(obj.first);
        args.emplace_back(obj.second);
    }
    return SafeSyncCommond("ZADD", args);
}

redisclient::RedisValue CRedisWrap::zcard(const std::string &key) {
    return SafeSyncCommond("ZCARD", {key});
}

redisclient::RedisValue CRedisWrap::zcount(const std::string &key, int min, int max) {
    return SafeSyncCommond("ZCOUNT", {key, min, max});
}

redisclient::RedisValue CRedisWrap::zincrby(const std::string &key, int inc, const std::string &member) {
    return SafeSyncCommond("ZINCRBY", {key, inc, member});
}

//redisclient::RedisValue zinterstore();
//redisclient::RedisValue zlexcount();
redisclient::RedisValue
CRedisWrap::zrange(const std::string &key, const std::string &start, const std::string &stop, bool withscores) {
    if (withscores) {
        return SafeSyncCommond("ZRANGE", {key, start, stop, "WITHSCORES"});
    } else {
        return SafeSyncCommond("ZRANGE", {key, start, stop});
    }
}

//redisclient::RedisValue zrangebylex(const std::string& key, const std::string& min, const std::string& max, const std::string& limit);
//redisclient::RedisValue zrangebyscore();
redisclient::RedisValue CRedisWrap::zrank(const std::string &key, const std::string &member) {
    return SafeSyncCommond("ZRANK", {key, member});
}

redisclient::RedisValue CRedisWrap::zrem(const std::string &key, const std::vector<std::string> &members) {
    std::deque<redisclient::RedisBuffer> args;
    args.emplace_back(key);
    for (const auto &obj:members) {
        args.emplace_back(obj);
    }
    return SafeSyncCommond("ZREM", args);
}

redisclient::RedisValue
CRedisWrap::zremrangebylex(const std::string &key, const std::string &min, const std::string &max) {
    return SafeSyncCommond("ZREMRANGEBYLEX", {key, min, max});
}

redisclient::RedisValue CRedisWrap::zremrangebyrank(const std::string &key, int start, int stop) {
    return SafeSyncCommond("ZREMRANGEBYRANK", {key, start, stop});
}

redisclient::RedisValue
CRedisWrap::zremrangebyscore(const std::string &key, const std::string &min, const std::string &max) {
    return SafeSyncCommond("ZREMRANGEBYSCORE", {key, min, max});
}

redisclient::RedisValue
CRedisWrap::zrevrange(const std::string &key, const std::string &start, const std::string &stop, bool withscores) {
    if (withscores) {
        return SafeSyncCommond("ZREVRANGE", {key, start, stop, "WITHSCORES"});
    } else {
        return SafeSyncCommond("ZREVRANGE", {key, start, stop});
    }
}

redisclient::RedisValue
CRedisWrap::zrevrangebylex(const std::string &key, const std::string &min, const std::string &max) {
    return SafeSyncCommond("ZREVRANGEBYLEX", {key, min, max});
}

//redisclient::RedisValue zrevrangebyscore();
redisclient::RedisValue CRedisWrap::zrevrank(const std::string &key, const std::string &member) {
    return SafeSyncCommond("ZREVRANK", {key, member});
}

//redisclient::RedisValue zscan();
redisclient::RedisValue CRedisWrap::zscore(const std::string &key, const std::string &member) {
    return SafeSyncCommond("ZSCORE", {key, member});
}
//redisclient::RedisValue zunionstore();

