#pragma once

#include <cassert>
#include <cstdint>
#include <functional>
#include <limits>
#include <mutex>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <unordered_set>
#include <uuid/uuid.h>
#include <vector>

namespace svrlib {
//uuid
namespace uuid {

    static const int8_t GUID_LEN = 64;

    static std::string generate()
    {
        char buf[GUID_LEN] = { 0 };
        uuid_t uu;
        uuid_generate(uu);
        int32_t index = 0;
        for (int32_t i = 0; i < 16; i++) {
            int32_t len = sprintf(buf + index, "%02X", uu[i]);
            if (len < 0)
                return std::move(std::string(""));
            index += len;
        }
        return std::move(std::string(buf));
    }
}

//进程内唯一id生成器
class puid {
public:
    puid(void)
        : id_(1) {};

    uint64_t get_id(void)
    {
        std::unique_lock<std::mutex> lock(lock_);
        if (0 == id_)
            id_ = 1;
        uint64_t ret = id_;
        ++id_;
        return ret;
    };

private:
    uint64_t id_;
    std::mutex lock_;
};

//ID生成器
template <typename T>
class IDGenerator final {
    static_assert(std::numeric_limits<T>::is_integer, "");

public:
    IDGenerator(T min_id = (std::numeric_limits<T>::min()), T max_id = (std::numeric_limits<T>::max()),
        size_t threshold = 4096)
        : min_(min_id)
        , max_(max_id)
        , next_(min_id)
        , threshold_(threshold)
    {
        assert(min_id <= max_id);
    }

    bool get(T& result)
    {
        if (pools_.size() >= threshold_) {
            result = *pools_.begin();
            pools_.erase(pools_.begin());
            return true;
        }

        if (next_ <= max_) {
            result = next_;
            ++next_;
            return true;
        }
        return false;
    }

    void put(T id)
    {
        auto found = pools_.find(id);
        assert(found == pools_.end());
        if (found == pools_.end()) {
            pools_.insert(id);
        }
    }

private:
    const T min_;
    const T max_;
    T next_;
    const size_t threshold_;
    std::unordered_set<T> pools_;
};

}
