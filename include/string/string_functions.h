
/***************************************************************
* 字符串函数
***************************************************************/
#pragma once

#include "utility/basic_types.h"
#include <string.h>
#include <stdarg.h>
/*************************************************************/

using namespace std;

extern void set_locale(const char* _locale);

extern const char* get_locale();

//------------------------------------------------------
//------------------------------ String Basic Functions
//--- 计算字符串长度
extern uint32_t dStrlen(const char* s);

//--- 字符串链接
#ifdef WIN32

extern int32_t dStrcat(char* dst, uint32_t _size, const char* src);

#else // WIN32
extern char*	dStrcat(char *dst,uint32_t _size, const char *src);
#endif // WIN32

//--- 字符串链接(_count:最多可链接数)
#ifdef WIN32

extern int32_t dStrncat(char* dst, uint32_t _size, const char* src, uint32_t _count);

#else // WIN32
extern char*	dStrncat(char *dst,uint32_t _size, const char *src,uint32_t _count);
#endif // WIN32

//--- 字符串拷贝
#ifdef WIN32

extern int32_t dStrcpy(char* dst, uint32_t _size, const char* src);

#else // WIN32
extern char*	dStrcpy(char *dst,uint32_t _size, const char *src);
#endif // WIN32

//--- 字符串拷贝(len:拷贝数)
#ifdef WIN32

extern int32_t dStrncpy(char* dst, uint32_t _size, const char* src, uint32_t len);

#else // WIN32
extern char*	dStrncpy(char *dst,uint32_t _size, const char *src, uint32_t len);
#endif // WIN32

//--- 拷贝字符串
extern int32_t dStrcpyMax(char* dst, uint32_t _size, const char* src, uint32_t len);

//--- 从buf所指内存区域的前count个字节查找字符ch位置
extern void* dMemchr(void* buf, char ch, uint32_t count);

//--- 字符串中查找第一次出现c的位置
extern char* dStrchr(char* s, char c);

extern const char* dStrchr(const char* s, char c);

//--- 分解字符串为一组标记串
extern char* dStrtok(char* s, const char* delim);

//--- 将字符串转换为小写形式
extern char* dStrlwr(char* s);

//--- 将字符串转换为大写形式
extern char* dStrupr(char* s);

//--- 是否宽字节
extern int32_t ismbblead(char c);

//--- 字符串比较(区分大小写)
extern int dStrncmp(const char* s1, const char* s2, int n);

//--- 字符串比较(不区分大小写)
extern int dStrnicmp(const char* s1, const char* s2, int n);

//--- 字符串查找第一次出现的位置(区分大小写)
extern char* dStrstr(char* haystack, char* needle);

//--- 字符串查找第一次出现的位置(不区分大小写)
extern char* dStristr(char* haystack, char* needle);

//--- 字符串格式化
extern int dSprintf(char* string, size_t sizeInBytes, const char* format, ...);

extern int dVsprintf(char* string, size_t sizeInBytes, const char* format, va_list _Args);

//--- 字符串打印
extern int dPrintf(const char* format, ...);

//------------------------------------------------------
//------------------------------ String Types

//--- 去除指定字符
extern void wipeOffChar(char* pStr, uint32_t uLen, char cChar);

//--- 过滤全半角空格
extern void filtrationBlank(char* pStr, uint32_t uLen);

//--- 是否是数字字符串
extern bool numeralString(const char* pStr, int32_t nLen);



