
#include "string/string_functions.h"
#include "utility/basic_functions.h"

#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32

#include <mbctype.h>
#include <windows.h>

#else
#endif // WIN32

#include <errno.h>
#include <memory.h>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

std::string g_strLocale = "zh_CN.utf8";

using namespace std;

/*************************************************************/
//-------------------------------------------------------------
//------------------------------
void set_locale(const char* _locale)
{
    if (_locale)
        g_strLocale = _locale;
    else
        g_strLocale.clear();
}

//-------------------------------------------------------------
//------------------------------
const char* get_locale()
{
    return g_strLocale.c_str();
}

//------------------------------------------------------
//------------------------------ 计算字符串长度
uint32_t dStrlen(const char* s)
{

    return ::strlen(s);
}

//------------------------------------------------------
//------------------------------ 字符串链接
#ifdef WIN32

int32_t dStrcat(char* dst, uint32_t _size, const char* src)
{

    return ::strcat_s(dst, _size, src);
}

#else // WIN32
char* dStrcat(char* dst, uint32_t _size, const char* src)
{
    return ::strcat(dst, src);
}
#endif // WIN32

//------------------------------------------------------
//------------------------------ 字符串链接(_count:最多可链接数)
#ifdef WIN32

int32_t dStrncat(char* dst, uint32_t _size, const char* src, uint32_t _count)
{

    return ::strncat_s(dst, _size, src, _count);
}

#else // WIN32
char* dStrncat(char* dst, uint32_t _size, const char* src, uint32_t _count)
{
    return ::strncat(dst, src, _count);
}
#endif // WIN32

//------------------------------------------------------
//------------------------------ 字符串拷贝
#ifdef WIN32

int32_t dStrcpy(char* dst, uint32_t _size, const char* src)
{

    return ::strcpy_s(dst, _size, src);
}

#else // WIN32
char* dStrcpy(char* dst, uint32_t _size, const char* src)
{
    return ::strcpy(dst, src);
}
#endif // WIN32

//------------------------------------------------------
//------------------------------ 字符串拷贝(len:拷贝数)
#ifdef WIN32

int32_t dStrncpy(char* dst, uint32_t _size, const char* src, uint32_t len)
{

    return ::strncpy_s(dst, _size, src, len);
}

#else // WIN323
char* dStrncpy(char* dst, uint32_t _size, const char* src, uint32_t len)
{
    return ::strncpy(dst, src, len);
}
#endif // WIN32

//-------------------------------------------------------------
//------------------------------ 拷贝字符串
extern int32_t dStrcpyMax(char* dst, uint32_t _size, const char* src, uint32_t len)
{
    if (len >= _size)
        len = _size - 1;

    memcpy(dst, src, len);

    return len;
}

//------------------------------------------------------
//------------------------------ 从buf所指内存区域的前count个字节查找字符ch位置
void* dMemchr(void* buf, char ch, uint32_t count)
{

    return ::memchr(buf, ch, count);
}

//------------------------------------------------------
//------------------------------ 字符串中查找第一次出现c的位置
char* dStrchr(char* s, char c)
{

    return ::strchr(s, c);
}

//-------------------------------------------------------------
//------------------------------
const char* dStrchr(const char* s, char c)
{

    return ::strchr(s, c);
}

//------------------------------------------------------
//------------------------------ 分解字符串为一组标记串
char* dStrtok(char* s, const char* delim)
{

    return ::strtok(s, delim);
}

//------------------------------------------------------
//------------------------------ 将字符串转换为小写形式
char* dStrlwr(char* s)
{

#ifdef WIN32
    return ::_strlwr(s);
#else // WIN32
    if (s) {
        char* cp;
        for (cp = s; *cp; ++cp) {
            if ('A' <= *cp && *cp <= 'Z')
                *cp += 'a' - 'A';
        }
    }
    return (s);
#endif // WIN32
}

//------------------------------------------------------
//------------------------------ 将字符串转换为大写形式
char* dStrupr(char* s)
{

#ifdef WIN32
    return ::_strupr(s);
#else // WIN32
    if (s) {
        char* cp;
        for (cp = s; *cp; ++cp) {
            if (('a' <= *cp) && (*cp <= 'z'))
                *cp -= 'a' - 'A';
        }
    }

    return (s);
#endif // WIN32
}

//-------------------------------------------------------------
//------------------------------ 是否宽字节
int32_t ismbblead(char c)
{
    /*是否宽字节*/
#ifdef WIN32
    return ::_ismbblead(c);
#else // WIN32
    return ((unsigned char)(c)&0x04);
    /*if(( (unsigned char )test_array[k]>=0xA1 && (unsigned char )test_array[k]<=0xF7)
        &&
        ((unsigned char )test_array[k+1]>=0xA1&&(unsigned char )test_array[k+1]<=0xFE))
    {
        if(begin==0)
            begin=k;
        tmp_array[i]=test_array[k];
        tmp_array[i+1]=test_array[k+1];
        k=k+2;
        i=i+2;
        have_chinese=1;
        if(flag==1)
            have_chinese=2;
    }*/
#endif // WIN32
}

//------------------------------------------------------
//------------------------------ 字符串比较(区分大小写)
int dStrncmp(const char* s1, const char* s2, int n)
{

    return ::strncmp(s1, s2, n);
}

//------------------------------------------------------
//------------------------------ 字符串比较(不区分大小写)
int dStrnicmp(const char* s1, const char* s2, int n)
{

#ifdef WIN32
    return ::_strnicmp(s1, s2, n);
#else // WIN32
    //return strcasecmp(s1,s2);
    if (s1 && s2 && n > 0) {
        int f = 0;
        int l = 0;

        do {

            if (((f = (unsigned char)(*(s1++))) >= 'A') && (f <= 'Z'))
                f -= 'A' - 'a';

            if (((l = (unsigned char)(*(s2++))) >= 'A') && (l <= 'Z'))
                l -= 'A' - 'a';

        } while (--n && f && (f == l));

        return (f - l);
    }
    return 0;
#endif // WIN32
}

//------------------------------------------------------
//------------------------------ 字符串查找第一次出现的位置(区分大小写)
char* dStrstr(char* haystack, char* needle)
{

    return ::strstr(haystack, needle);
}

//------------------------------------------------------
//------------------------------ 字符串查找第一次出现的位置(不区分大小写)
char* dStristr(char* haystack, char* needle)
{

    //strcasestr(haystack, needle);
    size_t len = ::dStrlen(needle);
    if (len == 0)
        return haystack; /* 这里我并未照strstr一样返回str，而是返回NULL*/

    while (*haystack) {
        /* 这里使用了可限定比较长度的strnicmp*/
        if (dStrnicmp(haystack, needle, len) == 0)
            return haystack;
        haystack++;
    }
    return NULL;
}

//-------------------------------------------------------------
//------------------------------ 字符串格式化
int dSprintf(char* string, size_t sizeInBytes, const char* format, ...)
{
    if (!string || !sizeInBytes || !format)
        return 0;

    int iRes = 0;
    va_list argptr;
    va_start(argptr, format);
#ifdef WIN32
    iRes = ::vsprintf_s(string, sizeInBytes, format, argptr);
#else // WIN32
    iRes = ::vsprintf(string, format, argptr);
#endif // WIN32
    va_end(argptr);

    return iRes;
}

//-------------------------------------------------------------
//------------------------------
int dVsprintf(char* string, size_t sizeInBytes, const char* format, va_list _Args)
{
    if (!string || !sizeInBytes || !format)
        return 0;

    int iRes = 0;
#ifdef WIN32
    iRes = ::vsprintf_s(string, sizeInBytes, format, _Args);
#else // WIN32
    iRes = ::vsprintf(string, format, _Args);
#endif // WIN32

    return iRes;
}

//-------------------------------------------------------------
//------------------------------ 字符串打印
int dPrintf(const char* format, ...)
{
    if (!format)
        return 0;

    int iRes = 0;
    va_list argptr;
    va_start(argptr, format);
#ifdef WIN32
    iRes = ::vprintf_s(format, argptr);
#else // WIN32
    iRes = ::vprintf(format, argptr);
#endif // WIN32
    va_end(argptr);

    return iRes;
}

//------------------------------------------------------
//------------------------------ 去除指定字符
void wipeOffChar(char* pStr, uint32_t uLen, char cChar)
{

    if (!pStr || !uLen)
        return;

    pStr[uLen] = 0;
    uint32_t uCount = 0;
    /*1、清除所有字符*/
    for (uint32_t i = 0; i < uLen; i++) {
        if (pStr[i] == 0)
            break;

        if (pStr[i] != cChar)
            continue;

        uCount++;
        pStr[i] = 0;
    }
    if (!uCount)
        return;

    /*2、移动*/
    uint32_t uMove = 0;
    for (uint32_t i = 0; i < uLen; i++) {
        if (uMove >= uCount)
            break;

        if (pStr[i] != 0)
            continue;

        uMove++;
        if (i != uLen - 1)
            memmove(pStr + i, pStr + i + 1, uLen - i - 1);
    }
    pStr[uLen - uCount] = 0;
}

//------------------------------------------------------
//------------------------------ 过滤全半角空格
void filtrationBlank(char* pStr, uint32_t uLen)
{

    if (!pStr || !uLen)
        return;

    pStr[uLen] = 0;

    /*1.将所有头部空格全部变为0*/
    for (uint32_t i = 0; i < uLen; i++) {
        /*遇到0跳出*/
        if (pStr[i] == 0)
            break;

        if (pStr[i] == ' ') {
            pStr[i] = 0;
            continue;
        }

        if (pStr[i] == -95) {
            if (i < uLen && pStr[i + 1] == -95) {
                pStr[i] = 0;
                pStr[++i] = 0;
                continue;
            }
        }

        /*1.将所有尾部空格全部变为0*/
        for (uint32_t j = uLen - 1; j > i; j--) {
            if (pStr[j] == 0)
                continue;

            if (pStr[j] == ' ') {
                pStr[j] = 0;
                continue;
            }
            if (pStr[j] == -95) {
                if (j > i && pStr[j - 1] == -95) {
                    pStr[j] = 0;
                    pStr[--j] = 0;
                    continue;
                }
            }
            break;
        }
        break;
    }

    /*3.将0之后的数据移动到前面来*/
    for (uint32_t i = 0; i < uLen; i++) {
        if (pStr[i] == 0)
            continue;

        memmove(pStr, pStr + i, uLen - i);
        break;
    }
}

//------------------------------------------------------
//------------------------------ 是否是数字字符串
bool numeralString(const char* pStr, int32_t nLen)
{

    if (!pStr || nLen <= 0)
        return false;

    for (int32_t i = 0; i < nLen; i++) {
        if (pStr[i] < '0' || pStr[i] > '9')
            return false;
    }

    return true;
}
