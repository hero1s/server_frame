/*----------------- net_encrypt.cpp
*
* Copyright (C): 2011  kuhx
* Author       : kuhx
* Version: 1.0
* Date: 2012/10/20 11:51:14
*--------------------------------------------------------------
*
*------------------------------------------------------------*/
#include "crypt/net_encrypt.h"

/*************************************************************/
Encrypt::Encrypt()
{
    m_key = 0;
}

/** 根据输入的密钥缓冲区，生成起始密钥 **/
void Encrypt::makeKey(const uint8_t* key, int32_t len)
{
    m_key = 0;
    for (int32_t i = 0; i < len; i++) {
        uint64_t k = key[i];
        uint64_t m = 1;
        k *= (m << (8 * (i % 8)));
        m_key += k;
    }
}

/** 根据当前的key值，生成下一个key值 **/
void Encrypt::makeNextKey()
{
    m_key = m_key * 0xE3779B97 + 0x9E3779B9;
}

/**
 * 设置加密使用的密钥
 * @param key 加密使用的密钥
 * @param len 密钥缓冲区长度
 * @author 康露 2012-10-11
 */
void Encrypt::setKey(const uint8_t* key, int32_t len)
{
    makeKey(key, len);
}

/**
 * 加密source指定的数据，每次加密都会导致密钥更新
 * @param source 需要被加密的数据缓冲区
 * @param len 需要被加密的数据缓冲区长度
 * @param dest 加密后内容输出缓冲区，这个缓冲区的大小必须大于len
 * @return 返回加密后内容输出缓冲区地址
 * @author 康露 2012-10-11
 */
uint8_t* Encrypt::encrypt(const uint8_t* source, int32_t len, uint8_t* dest)
{
    // 取m_key的最低位为掩码
    uint8_t mask = m_key & 0xff;
    dest[0] = (source[0] ^ mask);
    for (int32_t i = 1; i < len; i++) {
        dest[i] = (source[i] ^ mask);
        mask = source[mask % i];
    }
    makeNextKey();
    return dest;
}

/**
 * 解密source指定的内容，每次解密都会导致密钥更新
 * @param source 需要被解密的数据缓冲区
 * @param len 需要被解密的数据缓冲区
 * @param dest 解密后的数据缓冲区，这个缓冲区的大小必须大于len
 * @return 返回解密后的内容输出缓冲区地址
 * @author 康露 2012-10-11
 */
uint8_t* Encrypt::decrypt(const uint8_t* source, int32_t len, uint8_t* dest)
{
    uint8_t mask = m_key & 0xff;
    dest[0] = source[0] ^ mask;
    for (int32_t i = 1; i < len; i++) {
        dest[i] = (source[i] ^ mask);
        mask = dest[mask % i];
    }
    makeNextKey();
    return dest;
}
