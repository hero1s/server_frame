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

/** �����������Կ��������������ʼ��Կ **/
void Encrypt::makeKey(const uint8_t* key, int32_t len)
{
    m_key = 0;
    for (int32_t i = 0; i<len; i++)
    {
        uint64_t k = key[i];
        uint64_t m = 1;
        k *= (m << (8*(i%8)));
        m_key += k;
    }
}

/** ���ݵ�ǰ��keyֵ��������һ��keyֵ **/
void Encrypt::makeNextKey()
{
    m_key = m_key*0xE3779B97+0x9E3779B9;
}

/**
 * ���ü���ʹ�õ���Կ
 * @param key ����ʹ�õ���Կ
 * @param len ��Կ����������
 * @author ��¶ 2012-10-11
 */
void Encrypt::setKey(const uint8_t* key, int32_t len)
{
    makeKey(key, len);
}

/**
 * ����sourceָ�������ݣ�ÿ�μ��ܶ��ᵼ����Կ����
 * @param source ��Ҫ�����ܵ����ݻ�����
 * @param len ��Ҫ�����ܵ����ݻ���������
 * @param dest ���ܺ��������������������������Ĵ�С�������len
 * @return ���ؼ��ܺ����������������ַ
 * @author ��¶ 2012-10-11
 */
uint8_t* Encrypt::encrypt(const uint8_t* source, int32_t len, uint8_t* dest)
{
    // ȡm_key�����λΪ����
    uint8_t mask = m_key & 0xff;
    dest[0] = (source[0] ^ mask);
    for (int32_t i = 1; i<len; i++)
    {
        dest[i] = (source[i] ^ mask);
        mask = source[mask%i];
    }
    makeNextKey();
    return dest;
}

/**
 * ����sourceָ�������ݣ�ÿ�ν��ܶ��ᵼ����Կ����
 * @param source ��Ҫ�����ܵ����ݻ�����
 * @param len ��Ҫ�����ܵ����ݻ�����
 * @param dest ���ܺ�����ݻ�����������������Ĵ�С�������len
 * @return ���ؽ��ܺ�����������������ַ
 * @author ��¶ 2012-10-11
 */
uint8_t* Encrypt::decrypt(const uint8_t* source, int32_t len, uint8_t* dest)
{
    uint8_t mask = m_key & 0xff;
    dest[0] = source[0] ^ mask;
    for (int32_t i = 1; i<len; i++)
    {
        dest[i] = (source[i] ^ mask);
        mask = dest[mask%i];
    }
    makeNextKey();
    return dest;
}
