
/***************************************************************
* 
***************************************************************/
#include "helper/fileStream.h"
#include <stdarg.h>
#include <stdio.h>
/*************************************************************/
/*------------------------------------------------------------------------------
**
*/
const char    CFileStream::m_gszAccess[Access_Max][5] =
                      {
                              "rb",
                              "wb",
                              "rb+",
                              "ab+",
                      };

/*------------------------------------------------------------------------------
**
*/
CFileStream::CFileStream()
{
    m_pFile = NULL;
}

/*------------------------------------------------------------------------------
**
*/
CFileStream::~CFileStream()
{

    close();
}

/*------------------------------------------------------------------------------
**打开文件
*/
bool CFileStream::open(const char* pszFileName, _enAccess eAccess)
{
    if (eAccess>=Access_Max)
        eAccess = _Read;
    return open(pszFileName, m_gszAccess[eAccess]);
}

/*------------------------------------------------------------------------------
**打开文件
*/
bool CFileStream::open(const char* pszFileName, const char* pszMode)
{
    if (!pszFileName)
        return false;
    close();
#ifdef WIN32
    ::fopen_s(&m_pFile, pszFileName, pszMode ? pszMode : "rb");
#else//WIN32
    m_pFile = ::fopen(pszFileName,pszMode ? pszMode : "rb");
#endif//WIN32

    return (m_pFile!=NULL);
}

/*------------------------------------------------------------------------------
**关闭
*/
void CFileStream::close()
{

    if (m_pFile)
        ::fclose(m_pFile);

    m_pFile = NULL;
}

//-------------------------------------------------------------
//------------------------------ 更新缓冲区
void CFileStream::flush()
{
    //确保写入到文件-不一定是磁盘但是别处可以读取
    if (m_pFile)
        fflush(m_pFile);
}

/*------------------------------------------------------------------------------
**获得文件长度
*/
uint32_t CFileStream::getFileLength()
{

    if (!m_pFile)
        return 0;

    int32_t nTell = ftell(m_pFile);
    if (seek(0, SEEK_END)!=0)
    {
        seek(nTell, SEEK_SET);
        return 0;
    }

    uint32_t uLength = getPosition();
    seek(nTell, SEEK_SET);

    return uLength;
}

/*------------------------------------------------------------------------------
**获得当前文件指针位置
*/
uint32_t CFileStream::getPosition()
{

    if (!m_pFile)
        return 0;

    return ::ftell(m_pFile);
}

/*------------------------------------------------------------------------------
**清除文件
*/
int CFileStream::clear()
{

    if (!m_pFile)
        return 0;

    return ::fflush(m_pFile);
}

/*------------------------------------------------------------------------------
**是否已到文件尾
*/
bool CFileStream::isEof()
{

    if (!m_pFile)
        return true;

    return (::feof(m_pFile)!=0);
}

/*------------------------------------------------------------------------------
**偏移
**|SEEK_SET | SEEK_CUR		| SEEK_END	|
**|文件开头 | 当前读写的位置| 文件尾	|
*/
int CFileStream::seek(int32_t _n8Offset, int _Origin)
{

    if (!m_pFile)
        return 0;

    return ::fseek(m_pFile, _n8Offset, _Origin);
}

/*------------------------------------------------------------------------------
**读取
*/
bool CFileStream::_read(uint32_t uBytes, void* outBuffer)
{

    if (!m_pFile || !uBytes || !outBuffer)
        return 0;

    return (::fread(outBuffer, 1, uBytes, m_pFile)==uBytes);
}

/*------------------------------------------------------------------------------
**写入
*/
bool CFileStream::_write(uint32_t uBytes, const void* inBuffer)
{

    if (!m_pFile || !uBytes || !inBuffer)
        return 0;

    return (::fwrite(inBuffer, 1, uBytes, m_pFile)==uBytes);
}

/*------------------------------------------------------------------------------
**
*/
uint32_t CFileStream::fread(uint32_t uMaxBytes, void* outBuffer)
{

    if (!m_pFile || !uMaxBytes || !outBuffer)
        return 0;

    return ::fread(outBuffer, 1, uMaxBytes, m_pFile);
}

/*------------------------------------------------------------------------------
**
*/
uint32_t CFileStream::fwrite(uint32_t uMaxBytes, const void* inBuffer)
{

    if (!m_pFile || !uMaxBytes || !inBuffer)
        return 0;

    return ::fwrite(inBuffer, 1, uMaxBytes, m_pFile);
}

/*------------------------------------------------------------------------------
**
*/
uint32_t CFileStream::fprintf(const char* pszFormat, ...)
{

    if (!m_pFile || !pszFormat)
        return 0;

    va_list argptr;
    va_start(argptr, pszFormat);
#ifdef WIN32
    uint32_t uLen = ::vfprintf_s(m_pFile, pszFormat, argptr);
#else//WIN32
    uint32_t uLen = ::vfprintf(m_pFile,pszFormat,argptr);
#endif//WIN32
    va_end(argptr);

    return uLen;
}



