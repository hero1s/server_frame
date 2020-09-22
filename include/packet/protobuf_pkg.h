
#pragma once

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/message.h>
#include "svrlib.h"
#include "protobuf_handle.h"
#include "network/tcp_conn.h"

using namespace Network;

#pragma  pack(1)
//对外数据包头
typedef struct packet_header_t {
    uint16_t msgID;         // 消息id
} PACKETHEAD;

#define PACKET_MAX_SIZE             1024*16
#define PACKET_HEADER_SIZE          sizeof(packet_header_t)
#define PACKET_MAX_DATA_SIZE        (PACKET_MAX_SIZE - PACKET_HEADER_SIZE)

typedef struct {
    packet_header_t header;
    uint8_t protobuf[PACKET_MAX_DATA_SIZE];
} packet_protobuf;
#pragma pack()

class pkg_client {
public:
    static bool SendProtobufMsg(const TCPConnPtr &connPtr, const google::protobuf::Message *msg, uint16_t msgID) {
        static string sstr;
        msg->SerializeToString(&sstr);
        return SendBuffMsg(connPtr, sstr.c_str(), sstr.length(), msgID);
    }

    static bool SendBuffMsg(const TCPConnPtr &connPtr, const void *msg, uint16_t msg_len, uint16_t msgID) {
        if (connPtr == nullptr)
        {
            return false;
        }
        static packet_protobuf pkt;
        memset(&pkt, 0, sizeof(pkt));
        pkt.header.msgID = msgID;

        if (msg_len >= PACKET_MAX_DATA_SIZE)
        {
            LOG_ERROR("msg length more than max length:{}", msg_len);
            return false;
        }
        memcpy((void *) pkt.protobuf, msg, msg_len);

        //LOG_DEBUG("Socket Send Msg To Client-cmd:%d--len:%d",pkt.header.cmd,pkt.header.datalen);
        return connPtr->Send((char *) &pkt.header, msg_len + PACKET_HEADER_SIZE);
    }
};

// 消息处理
class CProtobufMsgHanlde : public CProtobufHandleBase<PACKETHEAD> {
public:
    int OnHandleClientMsg(const TCPConnPtr &connPtr, uint8_t *pData, size_t uiDataLen) {
        if (pData == NULL)
            return -1;
        packet_header_t *head = (packet_header_t *) pData;
        m_connPtr = connPtr;
        m_head = head;
        m_pkt_buf = pData + PACKET_HEADER_SIZE;
        m_buf_len = uiDataLen - PACKET_HEADER_SIZE;
        m_msgID = head->msgID;

        return OnRecvClientMsg();
    }
};







