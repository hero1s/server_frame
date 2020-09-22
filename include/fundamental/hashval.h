
#pragma once

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include "fundamental/common.h"

#define SHA_LEN 20
#define MD_LEN 16

static char HEX_DATA_MAP[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

template<int COUNT> //COUNT->闀垮害/sizeof(int)
struct HashVal {
    union {
      int     iVal[COUNT];
      uint8_t cVal[sizeof(int)*COUNT];
    };

    HashVal()
    {
        memset(iVal, 0, sizeof(iVal));
    }

    HashVal(const char* szHash)
    {
        UnHex(szHash);
    }

    HashVal(const uint8_t* pData, int len)
    {
        if (len==sizeof(cVal))
        {
            memcpy(cVal, pData, len);
        }
        else
        {
            memset(iVal, 0, sizeof(iVal));
        }
    }

    unsigned int GetInt() const
    {
        return (unsigned int) iVal[0];
    }

    uint32_t GetHashValue() const
    {
        unsigned int hash = 0;
        unsigned int x    = 0;
        const unsigned char* pch = cVal;
        unsigned int index = 0;
        while (index<sizeof(cVal))
        {
            hash   = (hash << 4)+(*pch++);
            if ((x = hash & 0xF0000000L)!=0)
            {
                hash ^= (x >> 24);
                hash &= ~x;
            }
            index++;
        }

        return (hash & 0x7FFFFFFF);
    }

    void UnHex(const char* szHash)
    {
        if (szHash && strlen(szHash)==sizeof(cVal)*2)
        {
            for (unsigned int i = 0; i<sizeof(cVal); ++i)
            {
                cVal[i] = HexStr2Val(szHash[i*2], szHash[i*2+1]);
            }
        }
        else
        {
            memset(iVal, 0, sizeof(iVal));
        }
    }

    const char* Hex() const
    {
        static char       buf[sizeof(cVal)*2+1];
        for (unsigned int i = 0; i<sizeof(cVal); ++i)
        {
            buf[i*2]   = HEX_DATA_MAP[(cVal[i] & 0xF0) >> 4];
            buf[i*2+1] = HEX_DATA_MAP[cVal[i] & 0xF];
        }
        buf[sizeof(cVal)*2] = '\0';
        return buf;
    }

    bool IsZero() const
    {
        for (unsigned int i = 0; i<sizeof(iVal)/sizeof(iVal[0]); ++i)
        {
            if (iVal[i]!=0) return false;
        }
        return true;
    }

    bool isValidID()
    {
        enum {
          MAX_TOTAL_ZERO  = 5, MAX_SERIATE_ZERO = 3,
          MAX_TOTAL_SPACE = 5, MAX_SERIATE_SPACE = 3
        };
        int  totalZero   = 0, totalSpace = 0;
        int  seriateZero = 1, seriateSpace = 1;
        char preChar     = 0, curChar;

        if (IsZero())
            return false;

        for (unsigned int i = 0; i<sizeof(cVal); i++)
        {
            curChar = cVal[i];
            if (curChar==0)
            {
                seriateSpace = 1;
                totalZero++;
                if (i>0 && preChar==0)
                    seriateZero++;
            }
            else if (curChar==' ')
            {
                seriateZero = 1;

                totalSpace++;
                if (i>0 && preChar==' ')
                    seriateSpace++;
            }
            else
            {
                seriateZero  = 1;
                seriateSpace = 1;
            }

            if ((totalZero>=MAX_TOTAL_ZERO || seriateZero>=MAX_SERIATE_ZERO)
                    || (totalSpace>=MAX_TOTAL_SPACE || seriateSpace>=MAX_SERIATE_SPACE))
                return false;

            preChar = curChar;
        }
        return true;
    }

    inline bool operator==(const HashVal& rhs) const
    {
        for (unsigned int i = 0; i<sizeof(iVal)/sizeof(iVal[0]); ++i)
        {
            if (iVal[i]!=rhs.iVal[i]) return false;
        }
        return true;
    }

    inline bool operator!=(const HashVal& rhs) const
    {
        return !operator==(rhs);
    }

    inline HashVal& operator=(const HashVal& rhs)
    {
        for (unsigned int i = 0; i<sizeof(iVal)/sizeof(iVal[0]); ++i)
        {
            iVal[i] = rhs.iVal[i];
        }
        return *this;
    }

    bool operator<(const HashVal& rhs) const
    {
        return memcmp(cVal, rhs.cVal, sizeof(cVal));
    }

    void Reset()
    {
        memset(iVal, 0, sizeof(iVal));
    }

    void ResetData(const uint8_t* pData, size_t len)
    {
        if (len==sizeof(cVal))
        {
            memcpy(cVal, pData, len);
        }
        else
        {
            memset(iVal, 0, sizeof(iVal));
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const HashVal& v)
    {
        os << std::hex;
        for (unsigned int i = 0; i<sizeof(v.cVal); ++i)
        {
            os << std::setw(2) << std::setfill('0') << (unsigned int) v.cVal[i];
        }
        os << std::dec;
        return os;
    }

private:
    uint8_t HexStr2Val(char c1, char c2)
    {
        return HexChar2Int(c1)*16+HexChar2Int(c2);
    }

    inline int HexChar2Int(char c)
    {
        if (c>='0' && c<='9') return c-'0';
        else if (c>='a' && c<='f') return c-'a'+10;
        else if (c>='A' && c<='F') return c-'A'+10;
        else return 0;
    }
};

typedef HashVal<16/sizeof(int)> Md5Hash;
typedef HashVal<20/sizeof(int)> Sha1Hash;


