
/***************************************************************
* 
***************************************************************/
#pragma once

#include <iostream>
#include <memory.h>
#include <vector>
#include "utility/basic_types.h"

/*************************************************************/

using namespace std;

#pragma pack(push, 1)

class CStreamBase {
public:
    virtual~CStreamBase() { }

public:
    virtual uint32_t getStreamSize()                        = 0;

    virtual uint32_t getPosition()                        = 0;

    virtual bool setPosition(uint32_t newPosition)    = 0;

protected:
    virtual bool _read(uint32_t uBytes, void* outBuffer)        = 0;

    virtual bool _write(uint32_t uBytes, const void* inBuffer)    = 0;

public:
    virtual uint32_t fprintf(const char* pszFormat, ...);

public:
    inline uint32_t getSpareSize() { return getStreamSize()-getPosition(); }

public:
    inline bool read(uint32_t uBytes, void* outBuffer) { return _read(uBytes, outBuffer); }

    inline bool write(uint32_t uBytes, const void* inBuffer) { return _write(uBytes, inBuffer); }

public:
    //------------------------------
    template<typename T>
    inline bool read(T* pBuffer) { return (pBuffer ? read(sizeof(T), pBuffer) : false); }

    //------------------------------
    template<typename T>
    inline bool read_(T& t) { return read(sizeof(T), &t); }

    //------------------------------
    template<typename T>
    inline bool write(const T* pBuffer) { return (pBuffer ? write(sizeof(T), pBuffer) : false); }

    //------------------------------
    template<typename T>
    inline bool write_(const T& t) { return write(sizeof(T), &t); }

    //------------------------------
    template<typename T>
    inline bool writeObj(const T& obj) { return obj.write(this); }

    //------------------------------
    template<typename T>
    inline bool readObj(const T& obj) { return obj.read(this); }

    //------------------------------
    template<typename T>
    inline bool readVector(vector<T>& vec)
    {
        uint32_t uCount = 0;
        if (!read_(uCount) || getSpareSize()<(sizeof(T)*uCount))
            return false;

        vec.resize(uCount);
        if (uCount && !read(sizeof(T)*uCount, &(vec[0])))
            return false;

        return true;
    }

    //------------------------------
    template<typename T>
    inline bool writeVector(vector<T>& vec)
    {
        uint32_t uCount = vec.size();
        if (!write_(uint32_t(uCount)))
            return false;

        if (uCount && !write(sizeof(T)*uCount, &(vec[0])))
            return false;

        return true;
    }

public:
    virtual bool isEof() { return (getSpareSize()==0); }

    virtual bool isEmpty() { return (getStreamSize()==0); }

public:
    virtual bool skipPosition(int32_t _Bytes);

    virtual bool readString(char* stringBuf, uint32_t bufferSize);

    virtual bool writeString(const char* stringBuf, int32_t maxLen = -1);

    virtual bool readLine(char* buffer, uint32_t bufferSize);

    virtual bool writeLine(const char* buffer);

    virtual bool writeString(const std::string& _string);

    virtual bool readString(std::string& _string);
};

#pragma pack(pop)



