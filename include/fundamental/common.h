
#pragma once
/**
 * In stdint.h, uint8_t, uint16,... and int8_t, int32_t... are defined.
 */
#include <stdint.h>
/**
 * defined size_t and ssize_t
 */
#include <unistd.h>

/**
 * for ntohl ...
 */
#include <arpa/inet.h>

#define RETURN_IF_NULL(pointer) {if(pointer == NULL) return;}
#define RETURN_NULL_IF_NULL(pointer) {if(pointer == NULL) return NULL;}
#define RETURN_FALSE_IF_NULL(pointer) {if(pointer == NULL) return false;}

#define RETURN_MINUS_ONE_IF_NULL(pointer) {if(pointer == NULL) return -1;}

#define THROW_IF_NULL(pointer, message) {if(pointer == NULL) { throw message;}}

inline size_t GetAlignedSize(size_t size)
{
    const int ALIGN = 8;
    return (size+ALIGN-1)/ALIGN*ALIGN;
}

inline uint64_t ntohll(uint64_t v)
{
    //本地序和网络序不一样
    if (ntohl(1)!=1)
    {
        uint64_t tempT = v;

        uint32_t* p = (uint32_t*) (&v);
        uint32_t* q = (uint32_t*) (&tempT);
        *p     = ntohl(*(q+1));
        *(p+1) = ntohl(*q);
    }
    return v;
}

inline uint64_t htonll(uint64_t v)
{
    return ntohll(v);
}

