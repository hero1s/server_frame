/*------------- urlEncode.h

*/
/***************************************************************
* url的编码与解码
* 如空格在url编码中为20%
***************************************************************/
#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <map>
/*************************************************************/
//#############################################################
//############################## url编码与解码
//#############################################################
namespace url_encode {
/*----->{ 编码 }*/
std::string& urlEncode(const std::string& str);

/*----->{ 解码 }*/
std::string& urlDecode(const std::string& str);
};

//#############################################################
//############################## http内容
//#############################################################
namespace http_content {
bool get_request(std::string& _request, const char* _host, unsigned short _port, const char* _url,
        const char* _content);

bool post_request(std::string& _request, const char* _host, unsigned short _port, const char* _url,
        unsigned int _length);

void response_head(std::string& _response, unsigned int _length, bool _utf8 = false);

bool is_http_message(const char* _buffer, unsigned short _size);

bool is_http_complete(const char* _buffer, unsigned short _size);
};

//#############################################################
//############################## url解析
//#############################################################
class url_parser : public std::map<std::string, std::string> {
protected:
    typedef std::map<std::string, std::string> Parent;

public:
    //--- 解析
    void param_parse(const char* _url);

    //--- 解析
    void param_parse(const char* _url, unsigned int _len);

    //--- 删除参数
    void param_del(const char* _key);

    //--- 增加参数
    void param_add(const char* _key, long long _data);

    //--- 增加参数
    void param_add(const char* _key, unsigned long long _data);

    //--- 增加参数
    void param_add(const char* _key, const char* _data, bool _must = false);

    //--- 返回字符串
    const char* param_to_url_string(const char* _prefix = NULL, bool _url_encode = true);

    //--- 获得参数整数
    long long get_param_long64(const char* _key);

    unsigned long long get_param_ulong64(const char* _key);

    //--- 获得参数字符串
    std::string* get_param_string(const char* _key);

    //--- 获得参数内容
    const char* get_param_content(const char* _key, const char* _empty = NULL);
};




