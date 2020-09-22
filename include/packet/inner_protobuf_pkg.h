//
// Created by Administrator on 2018/8/8.
//

#pragma once

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/message.h>
#include "svrlib.h"
#include "protobuf_handle.h"
#include "network/tcp_conn.h"

using namespace Network;

#pragma  pack(1)
//内部数据包头------------------------------------------------------------------------------------------------------------
typedef struct inner_header_t {
    uint8_t route;          // 路由类型
    uint32_t routeID;       // 路由ID
    uint32_t uin;           // uin(服务器内部转发用)
    uint16_t msgID;         // 消息id
} INNERHEAD;

#define INNER_MAX_SIZE             1024*32
#define INNER_HEADER_SIZE          sizeof(inner_header_t)
#define INNER_MAX_DATA_SIZE        (INNER_MAX_SIZE - INNER_HEADER_SIZE)

typedef struct {
    inner_header_t header;
    uint8_t protobuf[INNER_MAX_DATA_SIZE];
} inner_protobuf;

#pragma pack()

class pkg_inner {
public:
    static bool
    SendProtobufMsg(const TCPConnPtr& connPtr, const google::protobuf::Message *msg, uint16_t msgID, uint32_t uin,
                 uint8_t route, uint32_t routeID) {
        static string sstr;
        msg->SerializeToString(&sstr);
        return SendBuffMsg(connPtr, sstr.c_str(), sstr.length(), msgID, uin, route, routeID);
    }

    static bool
    SendBuffMsg(const TCPConnPtr& connPtr, const void *msg, uint16_t msg_len, uint16_t msgID, uint32_t uin,
                     uint8_t route, uint32_t routeID) {
        if (connPtr == nullptr) {
            LOG_ERROR("the connPtr is nullptr");
            return false;
        }
        static inner_protobuf pkt;
        memset(&pkt, 0, sizeof(pkt));
        pkt.header.msgID = msgID;
        pkt.header.uin = uin;
        pkt.header.route = route;
        pkt.header.routeID = routeID;

        if (msg_len >= INNER_MAX_DATA_SIZE) {
            LOG_ERROR("msg length more than max length:{}", msg_len);
            return false;
        }
        memcpy((void *) pkt.protobuf, msg, msg_len);
        //LOG_DEBUG("send msg:cmd:{},uid:{},len:{}",msg_type,uin,msg_len);
        return connPtr->Send((char*) &pkt.header, msg_len + INNER_HEADER_SIZE);
    }
};

// 消息处理
class CInnerMsgHanlde : public CProtobufHandleBase<INNERHEAD> {
public:

    int OnHandleClientMsg(TCPConnPtr connPtr, uint8_t *pData, size_t uiDataLen) {
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


