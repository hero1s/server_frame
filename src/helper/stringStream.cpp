
/***************************************************************
* 
***************************************************************/
#include <iostream>
#include <stdarg.h>
#include <stdlib.h>
#include "helper/stringStream.h"
#include "utility/basic_functions.h"
#include "string/string_functions.h"


/*************************************************************/
/*------------------------------------------------------------------------------
**
*/
CStringStream::CStringStream(uint32_t uLen)
{

    m_pszBuffer = NULL;
    m_pszPos    = NULL;
    initialize(uLen);
}

/*
**
*/
CStringStream::~CStringStream()
{

    if (m_pszBuffer)
        delete[] m_pszBuffer;
}

/*------------------------------------------------------------------------------
**
*/
void CStringStream::initialize(uint32_t uLen)
{

    m_uLength = uLen;
    m_pszPos  = NULL;
    if (m_pszBuffer)
        delete[] m_pszBuffer;

    if (m_uLength)
    {
        m_uLength++;
        m_pszBuffer = new char[m_uLength];
    }
    if (m_pszBuffer)
        memset(m_pszBuffer, 0, m_uLength);

    m_pszPos = m_pszBuffer;
}

/*------------------------------------------------------------------------------
**
*/
void CStringStream::reset()
{

    m_pszPos = m_pszBuffer;
}

/*------------------------------------------------------------------------------
**
*/
bool CStringStream::nexLine()
{

    if (!m_pszPos)
        return false;

    char* pszStr = strstr(m_pszPos, "\r\n");
    if (!pszStr)
        return false;

    if ((m_pszPos-m_pszBuffer)>=int32_t(m_uLength-3))
        return false;

    *pszStr   = '\0';
    *pszStr++ = '\0';
    m_pszPos = pszStr+1;

    return true;
}

/*------------------------------------------------------------------------------
**
*/
char* CStringStream::read(const char* pszFilt)
{

    if (m_pszPos==m_pszBuffer+(m_uLength-1))
        return NULL;

    char* pszStr = strstr(m_pszPos, pszFilt);
    if (!pszStr)
    {
        pszStr   = m_pszPos;
        m_pszPos = m_pszBuffer+(m_uLength-1);
        return pszStr;
    }

    *pszStr = '\0';
    char* pValue = m_pszPos;
    m_pszPos = pszStr+1;

    return pValue;
}

/*------------------------------------------------------------------------------
**
*/
bool CStringStream::_read(uint32_t uBytes, void* outBuffer)
{

    if (!uBytes || !outBuffer || getSpareSize()<uBytes)
        return false;

    memcpy(outBuffer, m_pszPos, uBytes);

    return skipPosition(uBytes);
}

/*------------------------------------------------------------------------------
**
*/
bool CStringStream::_write(uint32_t uBytes, const void* inBuffer)
{

    if (!uBytes || !inBuffer || getSpareSize()<uBytes)
        return false;

    memcpy(m_pszPos, inBuffer, uBytes);
    return skipPosition(uBytes);
}

/*------------------------------------------------------------------------------
**
*/
uint32_t CStringStream::fprintf(const char* pszFormat, ...)
{

    if (!m_pszPos || !pszFormat)
        return 0;

    va_list argptr;
    va_start(argptr, pszFormat);
    uint32_t uLen = dVsprintf(m_pszPos, size_t(getSpareSize()), pszFormat, argptr);
    va_end(argptr);

    skipPosition(uLen);

    return uLen;
}

/*------------------------------------------------------------------------------
**
*/
bool CStringStream::setPosition(uint32_t newPosition)
{

    if (m_uLength>newPosition)
        return false;

    m_pszPos = (m_pszBuffer+newPosition);

    return true;
}

/*------------------------------------------------------------------------------
**
*/
bool CStringStream::read(int32_t& uValue, const char* pszFilt)
{

    char* pszStr = read(pszFilt);
    if (!pszStr)
        return false;

    uValue = strtol(pszStr, NULL, 10);

    return true;
}

/*------------------------------------------------------------------------------
**
*/
bool CStringStream::read(uint32_t& uValue, const char* pszFilt)
{

    char* pszStr = read(pszFilt);
    if (!pszStr)
        return false;

    uValue = strtol(pszStr, NULL, 10);

    return true;
}

/*------------------------------------------------------------------------------
**
*/
bool CStringStream::read(int16_t& sValue, const char* pszFilt)
{

    char* pszStr = read(pszFilt);
    if (!pszStr)
        return false;

    sValue = (int16_t) strtol(pszStr, NULL, 10);

    return true;
}

/*------------------------------------------------------------------------------
**
*/
bool CStringStream::read(uint16_t& usValue, const char* pszFilt)
{

    char* pszStr = read(pszFilt);
    if (!pszStr)
        return false;

    usValue = (uint16_t) strtol(pszStr, NULL, 10);

    return true;
}

/*------------------------------------------------------------------------------
**
*/
bool CStringStream::read(int64_t& uValue, const char* pszFilt)
{

    char* pszStr = read(pszFilt);
    if (!pszStr)
        return false;

    uValue = strtoll(pszStr, NULL, 10);

    return true;
}

/*------------------------------------------------------------------------------
**
*/
bool CStringStream::read(uint64_t& uValue, const char* pszFilt)
{

    char* pszStr = read(pszFilt);
    if (!pszStr)
        return false;

    uValue = strtoull(pszStr, NULL, 10);

    return true;
}

/*------------------------------------------------------------------------------
**
*/
bool CStringStream::read(float& fValue, const char* pszFilt)
{

    char* pszStr = read(pszFilt);
    if (!pszStr)
        return false;
    fValue = (float) atof(pszStr);

    return true;
}

/*------------------------------------------------------------------------------
**
*/
bool CStringStream::read(int8_t& cValue, const char* pszFilt)
{

    char* pszStr = read(pszFilt);
    if (!pszStr)
        return false;
    cValue = (int8_t) strtol(pszStr, NULL, 10);

    return true;
}

/*------------------------------------------------------------------------------
**
*/
bool CStringStream::read(uint8_t& cValue, const char* pszFilt)
{

    char* pszStr = read(pszFilt);
    if (!pszStr)
        return false;
    cValue = (uint8_t) strtol(pszStr, NULL, 10);

    return true;
}

/*------------------------------------------------------------------------------
**
*/
bool CStringStream::read(char* pszString, uint32_t uLength, const char* pszFilt)
{

    if (!pszString || !uLength)
        return false;

    char* pszStr = read(pszFilt);
    if (!pszStr)
        return false;

    dStrcpy(pszString, uLength, pszStr);

    return true;
}


