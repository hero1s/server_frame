/*----------------- net_encrypt.h
*
* Copyright (C): 2012  kuhx
* Author       : kuhx
* Version: 1.0
* Date: 2012/10/20 11:51:32
*--------------------------------------------------------------
*
*------------------------------------------------------------*/
#pragma once

#include <iostream>
#include "utility/basic_types.h"
/*************************************************************/

/**
 * 网络加密算法类
 */
class Encrypt {
private:
    uint64_t m_key;

    /** 根据输入的密钥缓冲区，生成起始密钥 **/
    void makeKey(const uint8_t* key, int32_t len);

    /** 根据当前的key值，生成下一个key值 **/
    void makeNextKey();

public:
    Encrypt();

    /**
     * 设置加密使用的密钥
     * @param key 加密使用的密钥
     * @param len 密钥缓冲区长度
     * @author 康露 2012-10-11
     */
    void setKey(const uint8_t* key, int32_t len);

    /**
     * 加密source指定的数据，每次加密都会导致密钥更新
     * @param source 需要被加密的数据缓冲区
     * @param len 需要被加密的数据缓冲区长度
     * @param dest 加密后内容输出缓冲区，这个缓冲区的大小必须大于len
     * @return 返回加密后内容输出缓冲区地址
     * @author 康露 2012-10-11
     */
    uint8_t* encrypt(const uint8_t* source, int32_t len, uint8_t* dest);

    /**
     * 解密source指定的内容，每次解密都会导致密钥更新
     * @param source 需要被解密的数据缓冲区
     * @param len 需要被解密的数据缓冲区
     * @param dest 解密后的数据缓冲区，这个缓冲区的大小必须大于len
     * @return 返回解密后的内容输出缓冲区地址
     * @author 康露 2012-10-11
     */
    uint8_t* decrypt(const uint8_t* source, int32_t len, uint8_t* dest);
};

