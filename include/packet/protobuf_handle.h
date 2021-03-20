//
// Created by toney on 2019/2/1.
//
#pragma once

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/message.h>
#include "svrlib.h"
#include "network/tcp_conn.h"

using namespace Network;

using handFunc = function<int()>;

#ifndef PARSE_MSG
#define PARSE_MSG(msg)                   \
    if(ParseMsg(msg)<0){                 \
        LOG_DEBUG("PARSE_MSG fail");     \
        return -1;                       \
    }
#endif // PARSE_MSG

// 消息处理
template<typename Head>
class CProtobufHandleBase {
public:
    using handBase = CProtobufHandleBase<Head>;
    //  收到客户端消息时回调
    virtual int OnRecvClientMsg() {
        auto it = m_handlers.find(m_msgID);
        if (it != m_handlers.end()) {
            return it->second();
        }
        for(auto p:m_subHands){
            auto ret = p->OnDispatchMsg(m_connPtr,m_pkt_buf,m_buf_len,m_head);
            if(ret <= 0)return ret;
        }

        return 1;
    }

    //  添加消息映射
    template<class TYPE, class F>
    void bind_handler(TYPE *m, uint32_t key, F f) {
        m_handlers[key] = std::bind(f, m);
    }

    template<class MSGTYPE>
    int ParseMsg(MSGTYPE &msg) {
        if (!msg.ParseFromArray(m_pkt_buf, m_buf_len)) {
            LOG_ERROR("message unpack fail {}", msg.GetTypeName());
            return -1;
        }
        return 0;
    }
    void RegisterSubHandle(shared_ptr<handBase> hand){
        m_subHands.emplace_back(hand);
    }


    int OnDispatchMsg(const TCPConnPtr& connPtr, const uint8_t *pkt_buf,uint32_t buf_len,Head * head){
        m_connPtr = connPtr;
        m_head = head;
        m_pkt_buf = pkt_buf;
        m_buf_len = buf_len;
        m_msgID = head->msgID;
        return OnRecvClientMsg();
    }

protected:
    std::unordered_map<uint32_t, handFunc> m_handlers;
    TCPConnPtr m_connPtr;
    const uint8_t * m_pkt_buf;
    uint32_t m_buf_len;
    uint32_t m_msgID;
    Head * m_head;
private:
    std::vector<shared_ptr<handBase>> m_subHands;

};

