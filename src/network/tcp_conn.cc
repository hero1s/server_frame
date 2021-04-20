
#include "network/tcp_conn.h"
#include "crypt/base64.hpp"
#include "crypt/sha1.h"
#include "helper/bufferStream.h"
#include "memory/memory_pools.h"
#include "network/message_head.h"
#include "time/time.hpp"
#include "utility/comm_macro.h"
#include <arpa/inet.h>
#include <assert.h>

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#define WEB_SOCKET_HANDS_RE "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: %s\r\n\r\n"
#define MAGIC_KEY "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

namespace Network {
TCPConn::TCPConn(asio::io_service& service_, tcp::socket&& socket, std::string name, SocketParserType parserType)
    : io_service_(service_)
    , socket_(std::move(socket))
    , type_(kIncoming)
    , status_(kDisconnected)
    , name_(name)
    , local_ep_(socket_.local_endpoint())
    , remote_ep_(socket_.remote_endpoint())
    , recv_buffer_()
    , async_writing_(false)
    , write_buffer_()
    , writing_buffer_()
    , high_water_mark_(32 * 1024 * 1024)
    , conn_fn_(DefaultConnectionCallback)
    , msg_fn_(DefaultMessageCallback)
    , write_complete_fn_(nullptr)
    , high_water_mark_fn_(DefaultHighWaterMarkCallback)
    , close_fn_(nullptr)
{

    parserType_ = parserType;
    shake_hands_ = false;
    ws_head_.reset();
    SetKeepAlive(true);
    SetTCPNoDelay(true);
    decode_ = nullptr;
    InitHttpParser();
}

TCPConn::~TCPConn()
{
    //assert(status_ == kDisconnected);
    //assert(!socket_.is_open());
}

void TCPConn::Close()
{
    status_ = kDisconnecting;
    auto c = shared_from_this();

    if (c->socket_.is_open()) {
        asio::error_code ec;
        c->socket_.cancel(ec);
        c->socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
        c->socket_.close(ec);
    }
}

void TCPConn::HandleClose()
{
    if (status_ == kDisconnected) {
        return;
    }

    // close by peer
    if (socket_.is_open()) {
        asio::error_code ec;
        socket_.close(ec);
    }

    status_ = kDisconnecting;

    TCPConnPtr conn(shared_from_this());
    if (conn_fn_) {
        assert(status_ == kDisconnecting);
        conn_fn_(conn);
    }

    if (close_fn_) {
        close_fn_(conn);
    }
    status_ = kDisconnected;
}

void TCPConn::HandleError()
{
    status_ = kDisconnecting;
    HandleClose();
}

void TCPConn::SetHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t mark)
{
    high_water_mark_fn_ = cb;
    high_water_mark_ = mark;
}

void TCPConn::OnAttachedToLoop()
{
    status_ = kConnected;
    conn_fn_(shared_from_this());
    AsyncRead();
    recvtime_ = Now();
}

void TCPConn::SetTCPNoDelay(bool on)
{
    tcp::no_delay option(on);
    asio::error_code ec;
    socket_.set_option(option, ec);
    if (ec) {
        LOG_ERROR("set tcp no delay error:{}", on);
    }
}

void TCPConn::SetKeepAlive(bool on)
{
    asio::error_code ec;
    asio::socket_base::keep_alive op(true);
    socket_.set_option(op, ec);
    if (ec) {
        LOG_ERROR("set tcp keepalive error:{}", on);
    }
    {
        // 	int keepIdle = 60;			//开始首次KeepAlive探测前的TCP空闭时间(秒)
        typedef asio::detail::socket_option::integer<IPPROTO_TCP, TCP_KEEPIDLE> keep_aliveidle;
        socket_.set_option(keep_aliveidle(60), ec);
    }

    {
        // 	int keepInterval = 30;		//两次KeepAlive探测间的时间间隔（秒）
        typedef asio::detail::socket_option::integer<IPPROTO_TCP, TCP_KEEPINTVL> keep_aliveintvl;
        socket_.set_option(keep_aliveintvl(30), ec);
    }

    {
        // 	int keepCount = 3;			//判定断开前的KeepAlive探测次数
        typedef asio::detail::socket_option::integer<IPPROTO_TCP, TCP_KEEPCNT> keep_alivecnt;
        socket_.set_option(keep_alivecnt(3), ec);
    }
}

bool TCPConn::Send(const char* data, uint16_t sz)
{
    if (parserType_ == SocketParserType_WebSocket) {
        return SendWebSocketMsg(data, sz);
    }
    if (parserType_ == SocketParserType_Http) {
        return SendHttpMsg(string(data, sz), 200);
    }

    return SendInLoop(data, sz);
}

bool TCPConn::Send(const std::string& msg)
{
    if (parserType_ == SocketParserType_WebSocket) {
        return SendWebSocketMsg(msg.c_str(), msg.size());
    }
    if (parserType_ == SocketParserType_Http) {
        return SendHttpMsg(msg, 200);
    }
    return SendInLoop(msg.c_str(), msg.size());
}

void TCPConn::TimeOut(time_t now)
{
    if ((0 != timeout_) && (0 != recvtime_)) {
        if ((now - recvtime_ > timeout_)) {
            LOG_DEBUG("time out and close peer:{},{}", GetName(), timeout_);
            Close();
            return;
        }
        if (heart_msgcount_ > heart_maxmsg_) {
            LOG_DEBUG("recv msg more than max msg count:{} and close peer:{},{}", heart_msgcount_, GetName(), timeout_);
            Close();
            return;
        }
    }
    heart_msgcount_ = 0;
}

void TCPConn::SetHeartTimeOut(uint32_t second, uint32_t msg_num)
{
    timeout_ = second;
    heart_maxmsg_ = msg_num;
}

time_t TCPConn::Now()
{
    return svrlib::time::getSysTime();
}

bool TCPConn::SendInLoop(const char* data, uint16_t sz)
{
    if (!socket_.is_open()) {
        LOG_ERROR("the tcpConn is not open: {}", GetName());
        return false;
    }
    if (!async_writing_) {
        if (write_buffer_.Size() > 0) {
            writing_buffer_.Swap(write_buffer_);
        }
        writing_buffer_.Write(data, sz);
        AsyncWrite();
    } else {
        write_buffer_.Write(data, sz);
        size_t buffer_size = write_buffer_.Size() + writing_buffer_.Size();
        if (buffer_size > high_water_mark_) {
            if (high_water_mark_fn_) {
                high_water_mark_fn_(shared_from_this(), buffer_size);
            }
        }
    }
    return true;
}

bool TCPConn::SendWebSocketMsg(const char* data, uint16_t sz)
{
    if (!shake_hands_)
        return false;
    vector<char> buff;
    buff.reserve(sz + 16);
    buff.clear();
    auto stream = CBufferStream(buff.data(), buff.capacity());
    stream.write_((uint8_t)0x82); //写头部
    //写长度
    if (sz >= 126) { //7位放不下
        if (sz <= 0xFFFF) { //16位放
            stream.write_((uint8_t)126);
            stream.write_((uint16_t)htons((u_short)sz));
        } else { //64位放
            stream.write_((uint8_t)127);
            //stream.write_((uint64_t)OrderSwap64(wSize));
        }
    } else {
        stream.write_((uint8_t)sz);
    }
    //写数据
    stream.write(sz, data);
    SendInLoop(stream.getBuffer(), stream.getPosition());
    LOG_DEBUG("send web msg:{},size:{}", sz, stream.getPosition());
    return true;
}

bool TCPConn::SendHttpMsg(const std::string& msg, short result)
{
    size_t len = 1024 * 100;
    char* response = (char*)svrlib::CMemoryPools::Instance().GetBuff(len);
    memset(response, 0, len);
    int index = 0;
    int buff_size = len;

    index += snprintf(response, buff_size, "HTTP/1.1 %u \r\n", result);
    index += snprintf(&response[index], buff_size - index, "Content-Type: text/json\r\n");
    index += snprintf(&response[index], buff_size - index, "Content-Length: %d\r\n", int(msg.size()));
    index += snprintf(&response[index], buff_size - index, "\r\n");
    index += snprintf(&response[index], buff_size - index, "%s", msg.c_str());

    SendInLoop(response, index);

    svrlib::CMemoryPools::Instance().DelBuff(response);
    return true;
}

void TCPConn::AsyncRead()
{
    recv_buffer_.Normalize();
    recv_buffer_.EnsureValidSize();
    socket_.async_read_some(asio::buffer(recv_buffer_.WriteBegin(), recv_buffer_.ValidSize()),
        std::bind(&TCPConn::HandleRead, shared_from_this(), std::placeholders::_1,
            std::placeholders::_2));
}

void TCPConn::AsyncWrite()
{
    if (async_writing_ || writing_buffer_.Size() <= 0)
        return;
    async_writing_ = true;
    socket_.async_write_some(asio::buffer(writing_buffer_.ReadBegin(), writing_buffer_.Size()),
        std::bind(&TCPConn::HandleWrite, shared_from_this(), std::placeholders::_1,
            std::placeholders::_2));
}

void TCPConn::HandleRead(asio::error_code err, std::size_t trans_bytes)
{
    if (err) {
        if (err == asio::error::eof) {
            status_ = kDisconnecting;
            LOG_DEBUG("handle {} read eof:{}", GetName(), err.message());
            HandleClose();
            return;
        }
        LOG_DEBUG("handle {} read error:{}", GetName(), err.message());
        HandleError();
        return;
    }
    recv_buffer_.WriteBytes(trans_bytes);
    if (parserType_ == SocketParserType_WebSocket) {
        char* pPacket = nullptr;
        if (!shake_hands_) {
            pPacket = recv_buffer_.Data();
            ShakeHandsHandle(pPacket, recv_buffer_.Size());
            if (shake_hands_) {
                recv_buffer_.ReadBytes(recv_buffer_.Size());
                LOG_DEBUG("shake hand success,begin recv data");
                AsyncRead();
                return;
            } else {
                LOG_DEBUG("shake hand fail");
                AsyncRead();
                return;
            }
        }
        while (recv_buffer_.Size() > 0) {
            //LOG_DEBUG("recv msg:{}",recv_buffer_.Size());
            //读取websocket固定包头
            if (!ws_head_.rh) {
                //这个包不够一个头部的大小
                if (recv_buffer_.Size() < 2) {
                    //LOG_DEBUG("less head size:{}",recv_buffer_.Size());
                    break;
                }
                //读取
                uint8_t head = 0;
                recv_buffer_.Read_(head);
                ws_head_.fin = head >> 7;
                ws_head_.opcode = head & 0xF;
                recv_buffer_.Read_(head);
                ws_head_.len = head & 0x7F;
                ws_head_.mask = head >> 7;
                ws_head_.rh = 1; //标记头部读取完成
            }
            //读取长度
            if (!ws_head_.rl) {
                uint8_t nsize = ws_head_.GetLenNeedByte();
                if (nsize) {
                    //这个包不够一个长度
                    if (recv_buffer_.Size() < nsize) {
                        //LOG_DEBUG("less packet size:{} --- {}",recv_buffer_.Size(),nsize);
                        break;
                    }
                    if (nsize == 2) {
                        recv_buffer_.Read_(ws_head_.ex_len.v16);
                        ws_head_.ex_len.v16 = ntohs(ws_head_.ex_len.v16);
                    } else {
                        recv_buffer_.Read_(ws_head_.ex_len.v64);
                        ws_head_.ex_len.v64 = ntohl((u_long)ws_head_.ex_len.v64);
                    }
                }
                ws_head_.rl = 1;
            }
            //读取MKEY
            if (!ws_head_.rk) {
                if (ws_head_.mask) {
                    //这个包不够一个key
                    if (recv_buffer_.Size() < 4) {
                        //LOG_DEBUG("less key len:{}",recv_buffer_.Size());
                        break;
                    }
                    recv_buffer_.Read_(ws_head_.mkey[0]);
                    recv_buffer_.Read_(ws_head_.mkey[1]);
                    recv_buffer_.Read_(ws_head_.mkey[2]);
                    recv_buffer_.Read_(ws_head_.mkey[3]);
                }
                ws_head_.rk = 1;
            }
            //读取数据段
            uint64_t data_len = ws_head_.GetLen();
            if (recv_buffer_.Size() < data_len) {
                //LOG_DEBUG("data len is less:{},{}",recv_buffer_.Size(),data_len);
                break;
            }
            if (data_len > MAX_MESSAGE_LENGTH) {
                LOG_ERROR("the message length is more than length:{}", data_len);
                HandleClose();
                return;
            }

            ByteBuffer buf;
            buf.Write(recv_buffer_.ReadBegin(), data_len);
            recv_buffer_.ReadBytes(data_len);
            if (ws_head_.mask) {
                for (size_t i = 0; i < data_len; ++i) {
                    uint8_t tmp = buf.Data()[i];
                    tmp = tmp ^ ws_head_.mkey[i % 4];
                    buf.Data()[i] = tmp;
                }
            }
            //LOG_DEBUG("recv web msg:{}--len:{}",string((char*)(buf.Data()),buf.Size()),data_len);
            if (ws_head_.opcode == OPCODE_CLR) {
                LOG_DEBUG("websocket closed");
                HandleClose();
                break;
            }
            ws_head_.reset();
            msg_fn_(shared_from_this(), buf.Data(), buf.Size());
            recvtime_ = Now();
            heart_msgcount_++;
        }

    } else if (parserType_ == SocketParserType_Http) {
        auto nRet = http_parser_execute(&httpData.parser, &httpData.settings, recv_buffer_.ReadBegin(), recv_buffer_.Size());
        if (recv_buffer_.Size() != nRet) {
            return;
        } else if (httpData.http_status == HTTP_STATUS_MSG_COMPLETE) {
            httpData.reset();
        }
        recv_buffer_.ReadBytes(recv_buffer_.Size());
    } else {
        while (decode_ != nullptr) {
            int iLen = decode_->GetPacketLen(recv_buffer_.ReadBegin(), recv_buffer_.Size());
            if (iLen < 0) {
                LOG_ERROR("the message length is more than length:{}", recv_buffer_.Size());
                HandleClose();
                return;
            } else if (iLen > 0) {
                msg_fn_(shared_from_this(), recv_buffer_.ReadBegin(), iLen);
                recv_buffer_.ReadBytes(iLen);
                recvtime_ = Now();
                heart_msgcount_++;
            } else {
                break;
            }
        }
    }
    AsyncRead();
}

void TCPConn::HandleWrite(asio::error_code err, std::size_t trans_bytes)
{
    async_writing_ = false;

    if (err) {
        HandleError();
        return;
    }

    writing_buffer_.ReadBytes(trans_bytes);
    if (writing_buffer_.Size() > 0) {
        AsyncWrite();
        return;
    }
    writing_buffer_.Normalize();

    if (write_buffer_.Size() > 0) {
        writing_buffer_.Swap(write_buffer_);
        AsyncWrite();
        return;
    }
}

void TCPConn::ShakeHandsHandle(const char* buf, int buflen)
{
    char key[512];
    memset(key, 0, 512);
    for (int i = 0; i < buflen; ++i) {
        if (FindHttpParam("Sec-WebSocket-Key", buf + i)) {
            short k = i + 17, ki = 0;
            while (*(buf + k) != '\r' && *(buf + k) != '\n') {
                if (*(buf + k) == ':' || *(buf + k) == ' ') {
                    ++k;
                    continue;
                } else {
                    key[ki++] = *(buf + k);
                }
                ++k;
            }
            break;
        }
    }
    //LOG_DEBUG("key:{}...", key);
    memcpy(key + strlen(key), MAGIC_KEY, sizeof(MAGIC_KEY));
    //LOG_DEBUG("megerkey:{}...", key);
    //求哈希1
    SHA1 sha;
    unsigned int message_digest[5];
    sha.Reset();
    sha << key;
    sha.Result(message_digest);
    for (int i = 0; i < 5; i++) {
        message_digest[i] = htonl(message_digest[i]);
    }
    memset(key, 0, 512);
    svrlib::base64::encode(key, reinterpret_cast<const char*>(message_digest), 20);
    char http_res[640] = "";
    sprintf(http_res, WEB_SOCKET_HANDS_RE, key);
    SendInLoop((char*)http_res, strlen(http_res));
    shake_hands_ = true;
    LOG_DEBUG("shake hand success");
}

bool TCPConn::FindHttpParam(const char* param, const char* buf)
{
    while (*param == *buf) {
        if (*(param + 1) == '\0')
            return true;
        ++param;
        ++buf;
    }
    return false;
}
//------------------http--------------------//
int OnHttpHeaderComplete(http_parser* parser)
{
    TCPConn* httpHandler = (TCPConn*)parser->data;

    httpHandler->httpData.http_header_len = parser->nread;
    httpHandler->httpData.http_status = TCPConn::HTTP_STATUS_HEAD_COMPLETE;

    return 0;
}

int OnHttpMessageComplete(http_parser* parser)
{
    TCPConn* httpHandler = (TCPConn*)parser->data;

    httpHandler->httpData.http_body_len = parser->content_length;
    httpHandler->httpData.http_status = TCPConn::HTTP_STATUS_MSG_COMPLETE;

    return 0;
}

int OnHttpOnBodyProc(http_parser* parser, const char* data, size_t length)
{
    TCPConn* httpHandler = (TCPConn*)parser->data;
    if (httpHandler->OnHttpBodyPacketComplete(data, length) < 0) {
        return -1;
    }
    return 0;
}

void TCPConn::InitHttpParser()
{
    http_parser_settings_init(&httpData.settings);
    httpData.settings.on_headers_complete = OnHttpHeaderComplete;
    httpData.settings.on_message_complete = OnHttpMessageComplete;
    httpData.settings.on_body = OnHttpOnBodyProc;

    httpData.parser.data = this;
    http_parser_init(&httpData.parser, HTTP_REQUEST);
    httpData.reset();
}
int TCPConn::OnHttpBodyPacketComplete(const char* data, size_t len)
{
    msg_fn_(shared_from_this(), data, uint32_t(len));
    return 0;
}

};
