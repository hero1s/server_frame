
#pragma once

#include "asio.hpp"
#include "config/config.h"
#include "redisasyncclient.h"
#include "redissyncclient.h"
#include "svrlib.h"
#include <string>

using namespace std;
using namespace svrlib;

using reply_callback_t = std::function<void(redisclient::RedisValue v)>;

class CRedisWrap {
public:
    CRedisWrap();

    virtual ~CRedisWrap();

    virtual void OnTimer(const std::error_code& err);

    virtual bool Init(asio::io_context& context, const stRedisConf& conf);

    virtual void ShutDown();

    //常用命令
public:
    redisclient::RedisValue append(const std::string& key, const std::string& value);
    redisclient::RedisValue auth(const std::string& password);
    redisclient::RedisValue bgrewriteaof();
    redisclient::RedisValue bgsave();

    redisclient::RedisValue bitcount(const std::string& key);
    redisclient::RedisValue bitcount(const std::string& key, int start, int end);

    //hash
    redisclient::RedisValue hdel(const std::string& key, const std::vector<std::string>& fields);
    redisclient::RedisValue hexists(const std::string& key, const std::string& field);
    redisclient::RedisValue hget(const std::string& key, const std::string& field);
    redisclient::RedisValue hgetall(const std::string& key);
    redisclient::RedisValue hincrby(const std::string& key, const std::string& field, int incr);
    redisclient::RedisValue hincrbyfloat(const std::string& key, const std::string& field, float incr);
    redisclient::RedisValue hkeys(const std::string& key);
    redisclient::RedisValue hlen(const std::string& key);
    redisclient::RedisValue hmget(const std::string& key, const std::vector<std::string>& fields);
    redisclient::RedisValue hmset(const std::string& key, const std::vector<std::pair<std::string, std::string>>& field_vals);
    redisclient::RedisValue hset(const std::string& key, const std::string& field, const std::string& value);
    redisclient::RedisValue hscan(const std::string& key, const std::string& field, const std::string& value);
    redisclient::RedisValue hsetnx(const std::string& key, const std::string& field, const std::string& value);
    redisclient::RedisValue hstrlen(const std::string& key, const std::string& field);
    redisclient::RedisValue hvals(const std::string& key);

    //list
    redisclient::RedisValue blpop(const std::vector<std::string>& keys, int timeout);
    redisclient::RedisValue brpop(const std::vector<std::string>& keys, int timeout);
    redisclient::RedisValue brpoplpush(const std::string& src, std::string& dst, int timeout);
    redisclient::RedisValue lindex(const std::string& key, int index);
    redisclient::RedisValue linsert(const std::string& key, const std::string& before_after, const std::string& pivot, const std::string& value);
    redisclient::RedisValue llen(const std::string& key);
    redisclient::RedisValue lpop(const std::string& key);
    redisclient::RedisValue lpush(const std::string& key, const std::vector<std::string>& values);
    redisclient::RedisValue lpushx(const std::string& key, const std::string& value);
    redisclient::RedisValue lrange(const std::string& key, int start, int stop);
    redisclient::RedisValue lrem(const std::string& key, int count, const std::string& value);
    redisclient::RedisValue lset(const std::string& key, int index, const std::string& value);
    redisclient::RedisValue ltrim(const std::string& key, int start, int stop);
    redisclient::RedisValue rpop(const std::string& key);
    redisclient::RedisValue rpoplpush(const std::string& src, std::string& dst);
    redisclient::RedisValue rpush(const std::string& key, const std::vector<std::string>& values);
    redisclient::RedisValue rpushx(const std::string& key, const std::string& value);

    //set
    redisclient::RedisValue sadd(const std::string& key, const std::vector<std::string>& members);
    redisclient::RedisValue scard(const std::string& key);
    redisclient::RedisValue sdiff(const std::vector<std::string>& keys);
    redisclient::RedisValue sdiffstore(const std::string& dst, const std::vector<std::string>& keys);
    redisclient::RedisValue sinter(const std::vector<std::string>& keys);
    redisclient::RedisValue sinterstore(const std::string& dst, const std::vector<std::string>& keys);
    redisclient::RedisValue sismember(const std::string& key, const std::string& member);
    redisclient::RedisValue smembers(const std::string& key);
    redisclient::RedisValue smove(const std::string& src, const std::string& dst, const std::string& member);
    redisclient::RedisValue spop(const std::string& key, int count);
    redisclient::RedisValue srandmember(const std::string& key, int count);
    redisclient::RedisValue srem(const std::string& key, const std::vector<std::string>& members);
    //Redis& sscan;
    redisclient::RedisValue sunion(const std::vector<std::string>& keys);
    redisclient::RedisValue sunionstore(const std::string& dst, const std::vector<std::string>& keys);

    //sorted sets
    redisclient::RedisValue zadd(const std::string& key, const std::string& score, const std::string& member);
    redisclient::RedisValue zadd(const std::string& key, const std::vector<std::pair<std::string, std::string>>& score_members);
    redisclient::RedisValue zcard(const std::string& key);
    redisclient::RedisValue zcount(const std::string& key, int min, int max);
    redisclient::RedisValue zincrby(const std::string& key, int inc, const std::string& member);
    //redisclient::RedisValue zinterstore();
    //redisclient::RedisValue zlexcount();
    redisclient::RedisValue zrange(const std::string& key, const std::string& start, const std::string& stop, bool withscores);
    //redisclient::RedisValue zrangebylex(const std::string& key, const std::string& min, const std::string& max, const std::string& limit);
    //redisclient::RedisValue zrangebyscore();
    redisclient::RedisValue zrank(const std::string& key, const std::string& member);
    redisclient::RedisValue zrem(const std::string& key, const std::vector<std::string>& members);
    redisclient::RedisValue zremrangebylex(const std::string& key, const std::string& min, const std::string& max);
    redisclient::RedisValue zremrangebyrank(const std::string& key, int start, int stop);
    redisclient::RedisValue zremrangebyscore(const std::string& key, const std::string& min, const std::string& max);
    redisclient::RedisValue zrevrange(const std::string& key, const std::string& start, const std::string& stop, bool withscores);
    redisclient::RedisValue zrevrangebylex(const std::string& key, const std::string& min, const std::string& max);
    //redisclient::RedisValue zrevrangebyscore();
    redisclient::RedisValue zrevrank(const std::string& key, const std::string& member);
    //redisclient::RedisValue zscan();
    redisclient::RedisValue zscore(const std::string& key, const std::string& member);
    //redisclient::RedisValue zunionstore();

protected:
    virtual bool Reconnect(bool bSync, bool bAsync);

    virtual void TestConnect(bool bSync, bool bAsync);

    virtual void HandSyncError(const std::string& err);

    virtual void HandAsyncError(const std::string& err);

    virtual redisclient::RedisValue SafeSyncCommond(const std::string& cmd, std::deque<redisclient::RedisBuffer> args);

    virtual void SafeAsyncCommond(
        const std::string& cmd, std::deque<redisclient::RedisBuffer> args,
        std::function<void(redisclient::RedisValue)> handler = [](redisclient::RedisValue) {});

protected:
    std::shared_ptr<asio::system_timer> m_pTimer = nullptr;
    stRedisConf m_conf;
    std::shared_ptr<redisclient::RedisSyncClient> m_syncClient = nullptr;
    std::shared_ptr<redisclient::RedisAsyncClient> m_asyncClient = nullptr;

    bool m_asyncConnecting = false;
};
/*
    注意：connect,commond,pipeline命令报错会抛异常,pipe断线会有异常，暂时最好不用
*/
