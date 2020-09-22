
/***************************************************************
* 
***************************************************************/
#pragma once

#include "streamBase.h"

/*************************************************************/
class CStringStream : public CStreamBase {
public:
    uint32_t m_uLength;
    char* m_pszBuffer;
    char* m_pszPos;

public:
    CStringStream(uint32_t uLen = 0);

    virtual~CStringStream();

public:
    inline char*& getBuffer() { return m_pszBuffer; }

    inline uint32_t getLength() const { return m_uLength ? m_uLength-1 : m_uLength; }

public:
    void initialize(uint32_t uLen);

    void reset();

    bool nexLine();

    char* read(const char* pszFilt);

public:
    virtual uint32_t getStreamSize() { return getLength(); }

    virtual uint32_t getPosition() { return (m_pszPos-m_pszBuffer); }

    virtual bool isEof() { return (m_pszPos==m_pszBuffer+(m_uLength-1)); }

    virtual bool setPosition(uint32_t newPosition);

protected:
    virtual bool _read(uint32_t uBytes, void* outBuffer);

    virtual bool _write(uint32_t uBytes, const void* inBuffer);

public:
    virtual uint32_t fprintf(const char* pszFormat, ...);

public:
    bool read(int32_t& uValue, const char* pszFilt);

    bool read(uint32_t& uValue, const char* pszFilt);

    bool read(int16_t& sValue, const char* pszFilt);

    bool read(uint16_t& uValue, const char* pszFilt);

    bool read(int64_t& uValue, const char* pszFilt);

    bool read(uint64_t& uValue, const char* pszFilt);

    bool read(float& fValue, const char* pszFilt);

    bool read(int8_t& cValue, const char* pszFilt);

    bool read(uint8_t& cValue, const char* pszFilt);

    bool read(char* pszString, uint32_t uLength, const char* pszFilt);
};


