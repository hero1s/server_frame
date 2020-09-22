#pragma once

namespace svrlib
{
    class noncopyable
    {
    public:
        noncopyable() {}
        ~noncopyable() {}

        noncopyable(const noncopyable&) = delete;
        noncopyable& operator=(const noncopyable&) = delete;
    };
}


