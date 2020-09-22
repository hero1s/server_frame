
/***************************************************************
* 
***************************************************************/
#include <time.h>
#include "crypt/url_encode.h"
#include <stdlib.h>
#include <string>
#include <memory.h>

using namespace std;

/*************************************************************/
#if _MSC_VER>=1400 // VC++ 8.0
#pragma warning( disable : 4996 )   // disable warning about strdup being deprecated.
#endif
/*************************************************************/
//#############################################################
//############################## url编码与解码
//#############################################################
namespace url_encode {
char m_cHex[17] = "0123456789ABCDEF";

//-------------------------------------------------------------
//------------------------------ 编码
std::string& urlEncode(const std::string& str)
{
    static std::string dst;
    dst.clear();

    size_t      srcLen = str.size();
    for (size_t i      = 0; i<srcLen; i++)
    {
        unsigned char ch = str[i];
        if (isalnum(ch) || ch=='-' || ch=='_' || ch=='.')
        {/*是否为数字*/
            dst += ch;
            continue;
        }
        if (ch==' ')
        {/*是否为空格*/
            dst += '+';
        }
        else
        {/*字符串*/
            dst += '%';
            dst += m_cHex[ch/16];
            dst += m_cHex[ch%16];
        }
    }
    return dst;
}

//-------------------------------------------------------------
//------------------------------ 解码
std::string& urlDecode(const std::string& str)
{
    static std::string dst;
    dst.clear();

    size_t      srcLen = str.size();
    for (size_t i      = 0; i<srcLen; i++)
    {
        if (str[i]=='%')
        {
            if (isxdigit(str[i+1]) && isxdigit(str[i+2]))
            {
                char c1 = str[++i];
                char c2 = str[++i];
                c1 = c1-48-((c1>='A') ? 7 : 0)-((c1>='a') ? 32 : 0);
                c2 = c2-48-((c2>='A') ? 7 : 0)-((c2>='a') ? 32 : 0);
                dst += (unsigned char) (c1*16+c2);
            }
            continue;
        }

        if (str[i]=='+')
            dst += ' ';
        else
            dst += str[i];
    }

    return dst;
}
};
//#############################################################
//############################## http内容
//#############################################################
namespace http_content {
//-------------------------------------------------------------
//------------------------------
bool get_request(std::string& _request, const char* _host, unsigned short _port, const char* _url,
        const char* _content)
{
    if (!_host || !*_host)
        return false;

    //--- 指令
    _request = "GET ";
    ///utl
    if (_url && *_url)
        _request += _url;
    else
        _request += "/";

    ///内容
    if (_content && *_content)
    {
        _request += "?";
        _request += _content;
    }

    ///指令结束
    _request += " HTTP/1.1\r\n";

    //--- host:必须
    _request += "Host:";
    _request += _host;
    if (_port && _port!=80)
    {
        static char szText[32] = {0};

        sprintf(szText, ":%u", _port);
        _request += szText;
    }
    _request += "\r\n";

    //--- 链接属性
    _request += "Connection: close\r\n";

    //--- 包结束
    _request += "\r\n";

    return true;
}

//-------------------------------------------------------------
//------------------------------
bool post_request(std::string& _request, const char* _host, unsigned short _port, const char* _url,
        unsigned int _length)
{
    if (!_host || !*_host)
        return false;

    //--- 指令
    _request = "POST ";
    ///utl
    if (_url && *_url)
        _request += _url;
    else
        _request += "/";

    ///指令结束
    _request += " HTTP/1.1\r\n";

    //--- host:必须
    _request += "Host:";
    _request += _host;
    if (_port && _port!=80)
    {
        static char szText[32] = {0};

        sprintf(szText, ":%u", _port);
        _request += szText;
    }
    _request += "\r\n";

    //--- 内容长度
    static char szContentLength[64] = {0};
    sprintf(szContentLength, "Content-Length::%u\r\n", _length);
    _request += szContentLength;

    //--- 链接属性
    _request += "Connection: close\r\n";

    //--- 包头结束
    _request += "\r\n";

    return true;
}

//-------------------------------------------------------------
//------------------------------
void response_head(std::string& _response, unsigned int _length, bool _utf8)
{
    const char* month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    const char* week[]  = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
    static char buffer[64] = {0};

    memset(buffer, 0, sizeof(buffer));
    time_t t_now = ::time(NULL);
    tm* tm_now = ::localtime(&t_now);
    if (tm_now)
        sprintf(buffer, "Date: %s, %02d %s %4d %02d:%02d:%02dGMT\r\n", week[tm_now->tm_wday], tm_now->tm_mday,
                month[tm_now->tm_mon], tm_now->tm_year, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);

    _response = "HTTP/1.1 200 OK\r\n";
    _response += buffer;

    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "Content-Length: %d\r\n", _length);
    _response += buffer;

    _response += "Connection: Keep-Alive\r\n";
    _response += "Content-Type: text/html";
#ifdef WIN32
    if (_utf8)
        _response += ";charset=utf-8";
#else
    _response += ";charset=utf-8";
#endif
    _response += "\r\n\r\n";
}

//-------------------------------------------------------------
//------------------------------
bool is_http_message(const char* _buffer, unsigned short _size)
{
    if (!_buffer || _size<8)
        return false;

    if (memcmp(_buffer, "GET ", 4)==0)
        return true;

    if (memcmp(_buffer, "POST ", 5)==0)
        return true;

    return false;
}

//-------------------------------------------------------------
//------------------------------ 
bool is_http_complete(const char* _buffer, unsigned short _size)
{
    if (!_buffer || _size<8)
        return false;

    if (memcmp(_buffer, "GET ", 4)==0)
    {
        if (memcmp(&_buffer[_size-4], "\r\n\r\n", 4)==0)
            return true;
    }

    if (memcmp(_buffer, "POST ", 5)==0)
    {
        static const std::string _content_length = "Content-Length:";

        static std::string _utl;
        _utl.clear();

        _utl.append(_buffer, _size);

        string::size_type _find = _utl.find(_content_length);
        if (_find==std::string::npos)
            return false;

        unsigned int _length = atoi(_buffer+_find+_content_length.length());
        _find = _utl.find("\r\n\r\n");
        if (_find!=std::string::npos && (_size-_find-4)>=_length)
            return true;
    }

    return false;
}
};

//#############################################################
//############################## url解析
//#############################################################
//-------------------------------------------------------------
//------------------------------ 
void url_parser::param_parse(const char* _url)
{
    if (!_url || !*_url)
        return;

    param_parse(_url, strlen(_url));
}

//-------------------------------------------------------------
//------------------------------ 
void url_parser::param_parse(const char* _url, unsigned int _len)
{
    if (!_url || !*_url || !_len)
        return;

    static std::string strUrl;

    strUrl.clear();
    strUrl.append(_url, _len);

    string::size_type iFind = std::string::npos;
    std::string       strParam;
    std::string       strContent;
    while (!strUrl.empty())
    {
        //1分段参数
        iFind     = strUrl.find('&');
        if (iFind==std::string::npos)
            iFind = strUrl.length();

        strContent = strUrl.substr(0, iFind);
        strUrl.erase(0, iFind+1);

        //查找参数
        iFind = strContent.find('=');
        if (iFind==std::string::npos)
            break;

        strParam = strContent.substr(0, iFind);
        strContent.erase(0, iFind+1);

        strContent = url_encode::urlDecode(strContent);
        Parent::insert(Parent::value_type(strParam, strContent));
    }
}

//-------------------------------------------------------------
//------------------------------ 删除参数
void url_parser::param_del(const char* _key)
{
    if (!_key || !*_key)
        return;

    Parent::erase(_key);
}

//-------------------------------------------------------------
//------------------------------ 增加参数
void url_parser::param_add(const char* _key, long long _data)
{
    if (!_key || !*_key)
        return;

    char _data_[32] = {0};
    sprintf(_data_, "%lld", _data);
    param_add(_key, _data_);
}

//-------------------------------------------------------------
//------------------------------ 增加参数
void url_parser::param_add(const char* _key, unsigned long long _data)
{
    if (!_key || !*_key)
        return;

    char _data_[32] = {0};
    sprintf(_data_, "%llu", _data);
    param_add(_key, _data_);
}

//-------------------------------------------------------------
//------------------------------ 增加参数
void url_parser::param_add(const char* _key, const char* _data, bool _must)
{
    if (!_key || !*_key || (!_data && !_must))
        return;

    Parent::insert(Parent::value_type(_key, _data ? _data : ""));
}

//-------------------------------------------------------------
//------------------------------ 返回字符串
const char* url_parser::param_to_url_string(const char* _prefix, bool _url_encode)
{
    bool               _first = true;
    static std::string _request;
    _request.clear();
    Parent::iterator _pos = Parent::begin();
    Parent::iterator _end = Parent::end();
    for (; _pos!=_end; ++_pos)
    {
        const std::string& _key = _pos->first;
        if (_first)
        {
            _first = false;
            if (_prefix)
                _request += _prefix;
        }
        else
        {
            _request += "&";
        }
        _request += (_key+"="+(_url_encode ? url_encode::urlEncode(_pos->second) : _pos->second));
    }

    return _request.c_str();
}

//-------------------------------------------------------------
//------------------------------ 获得参数整数
long long url_parser::get_param_long64(const char* _key)
{
#ifdef WIN32
    return (long long) _atoi64(get_param_content(_key, "0"));
#else//WIN32
    return (long long)atoll(get_param_content(_key,"0"));
#endif//WIN32
}

//-------------------------------------------------------------
//------------------------------ 
unsigned long long url_parser::get_param_ulong64(const char* _key)
{
#ifdef WIN32
    return (unsigned long long) _strtoui64(get_param_content(_key, "0"), NULL, 10);
#else//WIN32
    return (unsigned long long)strtoull(get_param_content(_key,"0"),NULL,10);
#endif//WIN32
}

//-------------------------------------------------------------
//------------------------------ 获得参数字符串
std::string* url_parser::get_param_string(const char* _key)
{
    if (!_key || !*_key)
        return NULL;

    Parent::iterator pos = Parent::find(_key);
    if (pos!=Parent::end())
        return &(pos->second);

    return NULL;
}

//-------------------------------------------------------------
//------------------------------ 获得参数内容
const char* url_parser::get_param_content(const char* _key, const char* _empty)
{
    if (!_key || !*_key)
        return _empty;

    Parent::iterator pos = Parent::find(_key);
    if (pos!=Parent::end())
        return (pos->second).c_str();

    return _empty;
}


