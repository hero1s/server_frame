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
 * ��������㷨��
 */
class Encrypt {
private:
    uint64_t m_key;

    /** �����������Կ��������������ʼ��Կ **/
    void makeKey(const uint8_t* key, int32_t len);

    /** ���ݵ�ǰ��keyֵ��������һ��keyֵ **/
    void makeNextKey();

public:
    Encrypt();

    /**
     * ���ü���ʹ�õ���Կ
     * @param key ����ʹ�õ���Կ
     * @param len ��Կ����������
     * @author ��¶ 2012-10-11
     */
    void setKey(const uint8_t* key, int32_t len);

    /**
     * ����sourceָ�������ݣ�ÿ�μ��ܶ��ᵼ����Կ����
     * @param source ��Ҫ�����ܵ����ݻ�����
     * @param len ��Ҫ�����ܵ����ݻ���������
     * @param dest ���ܺ��������������������������Ĵ�С�������len
     * @return ���ؼ��ܺ����������������ַ
     * @author ��¶ 2012-10-11
     */
    uint8_t* encrypt(const uint8_t* source, int32_t len, uint8_t* dest);

    /**
     * ����sourceָ�������ݣ�ÿ�ν��ܶ��ᵼ����Կ����
     * @param source ��Ҫ�����ܵ����ݻ�����
     * @param len ��Ҫ�����ܵ����ݻ�����
     * @param dest ���ܺ�����ݻ�����������������Ĵ�С�������len
     * @return ���ؽ��ܺ�����������������ַ
     * @author ��¶ 2012-10-11
     */
    uint8_t* decrypt(const uint8_t* source, int32_t len, uint8_t* dest);
};

