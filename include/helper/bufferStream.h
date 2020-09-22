
/***************************************************************
* 
***************************************************************/
#pragma once

#include "streamBase.h"

/*************************************************************/
class CBufferStream : public CStreamBase {
private:
    char* m_pBuffer;        /*缓冲区指针*/
    uint32_t m_uBufferSize;    /*缓冲区大小*/
    uint32_t m_uBufferPos;    /*缓冲区当前位置*/

public:
    CBufferStream(char* pBuffer = NULL, uint32_t uSize = 0);

    virtual~CBufferStream();

public:
    //--- 构建一个全局静态数据库
    static CBufferStream& buildStream();

public:
    void initBufferStream(char* pBuffer, uint32_t uSize);

public:
    inline void initPosition() { m_uBufferPos = 0; }

    inline char* getBuffer() { return m_pBuffer; }

    inline char* getSpareBuffer() { return ((m_pBuffer && getSpareSize()) ? (m_pBuffer+getPosition()) : NULL); }

    inline bool writeStream(CBufferStream& clStream) { return _write(clStream.getPosition(), clStream.getBuffer()); }

    inline bool writeStreamSpare(CBufferStream& clStream)
    {
        return _write(clStream.getSpareSize(), clStream.getSpareBuffer());
    }

public:
    virtual uint32_t getStreamSize() { return m_uBufferSize; }

    virtual uint32_t getPosition() { return m_uBufferPos; }

    virtual bool setPosition(uint32_t newPosition);

protected:
    virtual bool _read(uint32_t uBytes, void* outBuffer);

    virtual bool _write(uint32_t uBytes, const void* inBuffer);

public:
    virtual uint32_t fprintf(const char* pszFormat, ...);

public:
    template<typename T>
    inline bool readPointer(T*& pPointer)
    {
        pPointer = NULL;
        if (getSpareSize()<sizeof(T))
            return false;

        pPointer = (T*) (getSpareBuffer());

        return skipPosition(sizeof(T));
    }
};



