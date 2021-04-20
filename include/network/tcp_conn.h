#pragma once

#include <asio.hpp>
#include <asio/ip/tcp.hpp>
#include <memory>

#include "byte_buffer.h"
#include "http_parser.h"
#include "tcp_callbacks.h"

using asio::ip::address;
using asio::ip::tcp;
namespace Network {
enum SocketParserType {
    SocketParserType_TCP = 0,
    SocketParserType_WebSocket = 1,
    SocketParserType_Http = 2,
};
class TCPConn
    : public std::enable_shared_from_this<TCPConn> {
public:
    enum Type {
        kIncoming = 0, // The type of a TCPConn held by a TCPServer
        kOutgoing = 1, // The type of a TCPConn held by a TCPClient
    };
    enum Status {
        kDisconnected = 0,
        kConnecting = 1,
        kConnected = 2,
        kDisconnecting = 3,
    };

    TCPConn(asio::io_service& service_, tcp::socket&& socket, std::string name, SocketParserType parserType = SocketParserType_TCP);

    virtual ~TCPConn();

    void Close();

    void SetMessageCallback(const MessageCallback& cb) { msg_fn_ = cb; }

    void SetMessageDecode(MsgDecodePtr decode) { decode_ = decode; }

    void SetConnCallback(const ConnCallback& cb) { conn_fn_ = cb; }

    void SetHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t mark);

    void SetCloseCallback(const CloseCallback& cb) { close_fn_ = cb; }

    void OnAttachedToLoop();

    std::string GetLocalAddress() const { return local_ep_.address().to_string(); }

    uint16_t GetLocalPort() const { return local_ep_.port(); }

    std::string GetRemoteAddress() const { return remote_ep_.address().to_string(); }

    uint16_t GetRemotePort() const { return remote_ep_.port(); }

    void SetTCPNoDelay(bool on);

    void SetKeepAlive(bool on);

    bool Send(const char* data, uint16_t sz);

    bool Send(const std::string& msg);

    bool IsConnected() const { return status_ == kConnected; }

    bool IsConnecting() const { return status_ == kConnecting; }

    bool IsDisconnected() const { return status_ == kDisconnected; }

    bool IsDisconnecting() const { return status_ == kDisconnecting; }

    void SetType(Type t) { type_ = t; }

    Type GetType() const { return type_; }

    bool IsIncommingConn() const { return type_ == kIncoming; }

    Status GetStatus() const { return status_; }

    std::string GetName() const { return name_; }

    void SetUID(uint32_t uid) { uid_ = uid; }

    uint32_t GetUID() { return uid_; }

    void TimeOut(time_t now);

    void SetHeartTimeOut(uint32_t second, uint32_t msg_num);

    static time_t Now();

protected:
    void HandleClose();

    void HandleError();

    void AsyncRead();

    void AsyncWrite();

    void HandleRead(asio::error_code err, std::size_t trans_bytes);

    void HandleWrite(asio::error_code err, std::size_t trans_bytes);

    bool SendInLoop(const char* data, uint16_t sz);

    bool SendWebSocketMsg(const char* data, uint16_t sz);

    //websocketЭ��
protected:
    enum WebSocketOpcode { //�����붨������
        OPCODE_MID = 0x0, //��ʶһ���м����ݰ�
        OPCODE_TXT = 0x1, //��ʶһ��text�������ݰ�
        OPCODE_BIN = 0x2, //��ʶһ��binary�������ݰ�
        //0x3 - 7������
        OPCODE_CLR = 0x8, //��ʶһ���Ͽ������������ݰ�
        OPCODE_PIN = 0x9, //��ʶһ��ping�������ݰ�
        OPCODE_PON = 0xA, //��ʾһ��pong�������ݰ�
    };

#pragma pack(push, 1)
    struct WebSocketHead {
        uint8_t fin : 1; //��ʶ�Ƿ�Ϊ����Ϣ�����һ�����ݰ�
        uint8_t rsv1 : 1; //����λ1
        uint8_t rsv2 : 1; //����λ2
        uint8_t rsv3 : 1; //����λ3
        uint8_t opcode : 4; //������

        uint8_t mask : 1; //�Ƿ���Ҫ����
        uint8_t len : 7; //����
        union {
            uint16_t v16; //����Ϊ126ʱ
            uint64_t v64; //����Ϊ127ʱ
        } ex_len;
        uint8_t mkey[4];
        uint8_t rh : 1; //head��ȡ���
        uint8_t rl : 1; //len��ȡ���
        uint8_t rk : 1; //mkey��ȡ���
        uint8_t rs : 5; //��չ����
        WebSocketHead(void) { reset(); }

        void reset(void) { memset(this, 0, sizeof(WebSocketHead)); }

        inline uint64_t GetLen(void)
        {
            if (len == 126) {
                return ex_len.v16;
            } else if (len == 127) {
                return ex_len.v64;
            }
            return len;
        }
        inline uint8_t GetLenNeedByte(void)
        {
            if (len == 126) {
                return 2;
            } else if (len == 127) {
                return 8;
            }
            return 0;
        }
    };

#pragma pack(pop)

private: //websocket
    void ShakeHandsHandle(const char* buf, int buflen);

    bool FindHttpParam(const char* param, const char* buf);

private:
    bool shake_hands_; // �Ƿ��Ѿ�����
    WebSocketHead ws_head_; // ��ͷ

public:
    //-------------------------http----------------------------------//
    enum HTTP_STATUS {
        HTTP_STATUS_INIT = 0,
        HTTP_STATUS_HEAD_COMPLETE,
        HTTP_STATUS_MSG_COMPLETE,
    };
    typedef struct httpMsgParser {
        void reset()
        {
            http_header_len = 0;
            http_body_len = 0;
            http_status = HTTP_STATUS_INIT;
        }

        http_parser_settings settings;
        http_parser parser;

        unsigned int http_header_len;
        unsigned int http_body_len;
        unsigned int http_status;
    } httpMsgParser;

    httpMsgParser httpData;
    void InitHttpParser();

    int OnHttpBodyPacketComplete(const char* data, size_t len);

    //-------------------------http---------------------------------//

private:
    asio::io_service& io_service_;
    tcp::socket socket_;
    SocketParserType parserType_;

    Type type_;
    Status status_;
    std::string name_;
    uint32_t uid_;

    tcp::endpoint local_ep_;
    tcp::endpoint remote_ep_;

    ByteBuffer recv_buffer_;
    time_t recvtime_ = 0;
    uint32_t timeout_ = 0;
    uint32_t heart_maxmsg_ = 0;
    uint32_t heart_msgcount_ = 0;

    bool async_writing_;
    ByteBuffer write_buffer_;
    ByteBuffer writing_buffer_;

    size_t high_water_mark_; // Default 32MB

    ConnCallback conn_fn_;
    MessageCallback msg_fn_;
    WriteCompleteCallback write_complete_fn_;
    HighWaterMarkCallback high_water_mark_fn_;
    CloseCallback close_fn_;
    MsgDecodePtr decode_;
};
};
