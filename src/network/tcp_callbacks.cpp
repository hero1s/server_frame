//
// Created by yinguohua on 2019/7/26.
//
#include "network/tcp_callbacks.h"
#include "network/tcp_conn.h"
#include "utility/comm_macro.h"

namespace Network {

void DefaultConnectionCallback(const TCPConnPtr& connPtr)
{
    LOG_DEBUG("connect ev:{} from:{}:{}", connPtr->GetName(), connPtr->GetRemoteAddress(), connPtr->GetRemotePort());
}

void DefaultMessageCallback(const TCPConnPtr& connPtr, char* pData, uint32_t length)
{
    LOG_DEBUG("recv {},msg from:{},{},size:{}", connPtr->GetName(), connPtr->GetRemoteAddress(), connPtr->GetRemotePort(), length);
}
void DefaultHighWaterMarkCallback(const TCPConnPtr& connPtr, uint32_t len)
{
    LOG_ERROR("high water mark :{},ip:{},size:{}", connPtr->GetName(), connPtr->GetRemoteAddress(), len);
    connPtr->Close();
}

};
