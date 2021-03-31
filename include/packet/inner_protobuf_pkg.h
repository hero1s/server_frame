//
// Created by Administrator on 2018/8/8.
//

#pragma once

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/message.h>
#include "svrlib.h"
#include "protobuf_handle.h"
#include "network/tcp_conn.h"
#include "network/message_head.h"

using namespace Network;

#pragma  pack(1)
//内部数据包头------------------------------------------------------------------------------------------------------------
typedef struct inner_header_t {
    uint32_t msgLen;        // 消息长度
    uint16_t msgID;         // 消息id
    uint32_t uid;           //
    uint32_t s_ser_id;      // 源服务器
    uint8_t s_ser_type;    // 源服务器类型
    uint32_t d_ser_id;      // 目标服务器
    uint8_t d_ser_type;    // 目标服务器类型
} INNERHEAD;

#define INNER_MAX_SIZE             1024*32
#define INNER_HEADER_SIZE          sizeof(inner_header_t)
#define INNER_MAX_DATA_SIZE        (INNER_MAX_SIZE - INNER_HEADER_SIZE)

typedef struct {
    inner_header_t header;
    uint8_t protobuf[INNER_MAX_DATA_SIZE];
} inner_protobuf;

#pragma pack()

class InnerDecode : public MsgDecode {
public:
    //0收包不完全，>0 获取收到的包长，-1 错误包
    virtual int GetPacketLen(const char *pData, uint32_t len) {
        if (len < sizeof(inner_header_t)) {
            return 0;
        }
        inner_header_t *head = (inner_header_t *) pData;
        if (head->msgLen > INNER_MAX_SIZE) {
            LOG_ERROR("head msg len:{}", head->msgLen);
            return -1;
        }
        if (head->msgLen < len) {
            return head->msgLen;
        }
        return 0;
    }
};

class pkg_inner {
public:
    static bool
    SendProtobufMsg(const TCPConnPtr &connPtr, const google::protobuf::Message *msg, uint16_t msgID, uint32_t uid,
                    uint8_t s_ser_type, uint32_t s_ser_id, uint8_t d_ser_type, uint32_t d_ser_id) {
        static string sstr;
        msg->SerializeToString(&sstr);
        return SendBuffMsg(connPtr, sstr.c_str(), sstr.length(), msgID, uid, s_ser_type, s_ser_id,d_ser_type,d_ser_id);
    }

    static bool
    SendBuffMsg(const TCPConnPtr &connPtr, const void *msg, uint32_t msg_len, uint16_t msgID, uint32_t uid,
                uint8_t s_ser_type, uint32_t s_ser_id, uint8_t d_ser_type, uint32_t d_ser_id) {
        if (connPtr == nullptr) {
            LOG_ERROR("the connPtr is nullptr");
            return false;
        }
        static inner_protobuf pkt;
        memset(&pkt, 0, sizeof(pkt));
        pkt.header.msgID = msgID;
        pkt.header.uid = uid;
        pkt.header.s_ser_type = s_ser_type;
        pkt.header.s_ser_id = s_ser_id;
        pkt.header.d_ser_type = d_ser_type;
        pkt.header.d_ser_id = d_ser_id;
        pkt.header.msgLen = msg_len + INNER_HEADER_SIZE;

        if (msg_len >= INNER_MAX_DATA_SIZE) {
            LOG_ERROR("msg length more than max length:{}", msg_len);
            return false;
        }
        memcpy((void *) pkt.protobuf, msg, msg_len);
        //LOG_DEBUG("send msg:cmd:{},uid:{},len:{}",msg_type,uin,msg_len);
        return connPtr->Send((char *) &pkt.header, msg_len + INNER_HEADER_SIZE);
    }
};

// 消息处理
class CInnerMsgHanlde : public CProtobufHandleBase<INNERHEAD> {
public:

    int OnHandleClientMsg(TCPConnPtr connPtr, uint8_t *pData, uint32_t uiDataLen) {
        if (pData == NULL)
            return -1;
        inner_header_t *head = (inner_header_t *) pData;
        m_connPtr = connPtr;
        m_head = head;
        m_pkt_buf = pData + INNER_HEADER_SIZE;
        m_buf_len = uiDataLen - INNER_HEADER_SIZE;
        m_msgID = head->msgID;

        return OnRecvClientMsg();
    }
};


