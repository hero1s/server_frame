
#pragma once

#include <vector>
#include <map>
#include <time.h>
#include <random>
#include <functional>
#include <random>
#include <string>
#include <cassert>
#include "utility/basic_types.h"

namespace svrlib
{
    ///[min.max]
    template<typename IntType>
    inline IntType rand_range(IntType min, IntType max)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<IntType> dis(min, max);
        return dis(gen);
    }

    inline unsigned int rand()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<unsigned int> dis(1, std::numeric_limits<unsigned int>::max());
        return dis(gen);
    }

    inline const unsigned char* randkey(size_t len = 8)
    {
        static thread_local unsigned char tmp[256];
        assert(len < sizeof(tmp));
        char x = 0;
        for (size_t i = 0; i < len; ++i)
        {
            tmp[i] = static_cast<unsigned char>(rand() & 0xFF);
            x ^= tmp[i];
        }

        if (x == 0)
        {
            tmp[0] |= 1;// avoid 0
        }

        return tmp;
    }

    ///[min,max)
    template< class RealType = double >
    inline RealType randf_range(RealType min, RealType max)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<RealType> dis(min, max);
        return dis(gen);
    }

    template<typename Key, typename Weight>
    inline Key rand_weight(const std::map<Key, Weight>& data)
    {
        Weight total = 0;
        for (auto& it : data)
        {
            total += it.second;
        }
        if (total == 0)
        {
            return 0;
        }
        Weight rd = rand(1, total);

        Weight tmp = 0;
        for (auto& it : data)
        {
            tmp += it.second;
            if (rd <= tmp)
            {
                return it.first;
            }
        }
        return 0;
    }
};






