
/***************************************************************
* 
***************************************************************/
#include "helper/streamBase.h"
#include <iostream>
/*************************************************************/
/*------------------------------------------------------------------------------
**
*/
uint32_t CStreamBase::fprintf(const char* pszFormat, ...)
{
    return 0;
}

/*------------------------------------------------------------------------------
**
*/
bool CStreamBase::skipPosition(int32_t _Bytes)
{

    if (_Bytes==0)
        return true;

    //鍚戝墠
    if (_Bytes<0)
        return ((getPosition()>=uint32_t(-1*_Bytes)) ? setPosition(getPosition()+_Bytes) : false);

    return ((getSpareSize()>=(uint32_t) _Bytes) ? setPosition(getPosition()+_Bytes) : false);
}

/*------------------------------------------------------------------------------
**
*/
bool CStreamBase::readString(char* stringBuf, uint32_t bufferSize)
{

    if (!stringBuf || !bufferSize)
        return false;

    uint32_t uLen = 0;
    if (!read(&uLen))
        return false;

    if (uLen>=bufferSize)
    {
        read(bufferSize, stringBuf);
        skipPosition(uLen-bufferSize);

        stringBuf[bufferSize] = '\0';
    }
    else
    {
        read(uLen, stringBuf);
        stringBuf[uLen] = '\0';
    }
    return true;
}

/*------------------------------------------------------------------------------
**
*/
bool CStreamBase::writeString(const char* stringBuf, int32_t maxLen)
{

    if (!stringBuf || maxLen==0)
        return false;

    uint32_t uLen = strlen(stringBuf);
    if (maxLen>0 && uint32_t(maxLen)>uLen)
        uLen = maxLen;

    if (!write(&uLen))
        return false;

    return write(uLen, stringBuf);
}

/*------------------------------------------------------------------------------
**
*/
bool CStreamBase::readLine(char* buffer, uint32_t bufferSize)
{

    if (!buffer || !bufferSize)
        return false;

    bufferSize--;
    char* buff    = buffer;
    char* buffEnd = buffer+bufferSize;
    *buff = '\r';

    uint32_t uLen = 0;
    while (*buff=='\r')
    {
        if (!read(buff) || *buff=='\n')
        {
            *buff = 0;
            return false;
        }
        uLen++;
    }

    while (buff!=buffEnd && read(++buff) && *buff!='\n')
    {
        uLen++;
        if (*buff=='\r')
            buff--;
    }
    *buff = 0;

    return true;
}

/*------------------------------------------------------------------------------
**
*/
bool CStreamBase::writeLine(const char* buffer)
{

    if (!write(strlen(buffer), buffer) || !write(2, "\r\n"))
        return false;

    return true;
}

//-------------------------------------------------------------
//------------------------------ 
bool CStreamBase::writeString(const std::string& _string)
{
    if (!write_(uint32_t(_string.length())))
        return false;

    if (_string.length()>0 && !write(_string.length(), _string.c_str()))
        return false;

    return true;
}

//-------------------------------------------------------------
//------------------------------ 
bool CStreamBase::readString(std::string& _string)
{
    _string.clear();
    uint32_t uLength = 0;
    if (!read_(uLength))
        return false;

    const uint32_t uBufferSize           = 1024;
    char         szBuffer[uBufferSize] = {0};

    uint32_t uSize = 0;
    while (uLength)
    {
        if (uLength>uBufferSize)
            uSize = uBufferSize;
        else
            uSize = uLength;

        if (uLength>uSize)
            uLength -= uSize;
        else
            uLength = 0;

        if (!read(uSize, szBuffer))
            return false;

        _string.append(szBuffer, uSize);
    }

    return true;
}


