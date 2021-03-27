/*
 * Copyright (C) Alex Nekipelov (alex@nekipelov.net)
 * License: MIT
 */


#ifndef REDISSYNCCLIENT_REDISBUFFER_H
#define REDISSYNCCLIENT_REDISBUFFER_H

#include <variant>

#include <string>
#include <vector>

#include "config.h"

namespace redisclient {

struct RedisBuffer
{
    RedisBuffer() = default;
    inline RedisBuffer(const char *ptr, size_t dataSize);
    inline RedisBuffer(const char *s);
    inline RedisBuffer(std::string s);
    inline RedisBuffer(std::vector<char> buf);
    inline RedisBuffer(int64_t l);//modify toney
    inline RedisBuffer(float f);
    inline RedisBuffer(int i);
    inline RedisBuffer(double d);


    inline size_t size() const;

    std::variant<std::string,std::vector<char>> data;
};


RedisBuffer::RedisBuffer(const char *ptr, size_t dataSize)
    : data(std::vector<char>(ptr, ptr + dataSize))
{
}

RedisBuffer::RedisBuffer(const char *s)
    : data(std::string(s))
{
}

RedisBuffer::RedisBuffer(std::string s)
    : data(std::move(s))
{
}

RedisBuffer::RedisBuffer(std::vector<char> buf)
    : data(std::move(buf))
{
}
RedisBuffer::RedisBuffer(int64_t l)
    :data(std::to_string(l))
{

}
RedisBuffer::RedisBuffer(float f)
        :data(std::to_string(f))
{

}
RedisBuffer::RedisBuffer(int i)
        :data(std::to_string(i))
{

}
RedisBuffer::RedisBuffer(double d)
        :data(std::to_string(d))
{

}

size_t RedisBuffer::size() const
{
    if (std::holds_alternative<std::string>(data))
        return std::get<std::string>(data).size();
    else
        return std::get<std::vector<char>>(data).size();
}

}

#endif //REDISSYNCCLIENT_REDISBUFFER_H

