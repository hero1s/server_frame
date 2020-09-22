
/***************************************************************
* 
***************************************************************/
#pragma once

#include "streamBase.h"

/*************************************************************/
class CFileStream : public CStreamBase {
public:
    enum _enAccess {
      _Read,/*读*/
      _Write,/*写*/
      _ReadWrite,/*读写*/
      _Append,/*追加*/
      Access_Max
    };

protected:
    static const char m_gszAccess[Access_Max][5];

protected:
    FILE* m_pFile;

public:
    inline bool isOpen() { return (m_pFile!=NULL); }

    inline FILE* getFile() { return m_pFile; }

public:
    CFileStream();

    virtual~CFileStream();

public:
    /*----->{ 打开文件 }*/
    bool open(const char* pszFileName, _enAccess eAccess);

    /*----->{ 打开文件 }*/
    bool open(const char* pszFileName, const char* pszMode = "rb");

    /*----->{ 关闭 }*/
    void close();

    //--- 更新缓冲区
    void flush();

public:
    /*----->{ 获得文件长度 }*/
    uint32_t getFileLength();

    /*----->{ 清除文件 }*/
    int clear();

    /*----->{ 偏移(成功返回0) }*/
    int seek(int32_t _nOffset, int _Origin = SEEK_SET);

public:
    /*----->{ 从文件头偏移 }*/
    inline int seekBegin(int32_t _nOffset) { return seek(_nOffset, SEEK_SET); }

    /*----->{ 从文件尾偏移 }*/
    inline int seekEnd(int32_t _nOffset) { return seek(_nOffset, SEEK_END); }

    /*----->{ 从文件当前位置偏移 }*/
    inline int seekCurrent(int32_t _nOffset) { return seek(_nOffset, SEEK_CUR); }

public:
    virtual uint32_t getStreamSize() { return getFileLength(); }

    virtual uint32_t getPosition();

    virtual bool setPosition(uint32_t newPosition) { return (seekBegin(newPosition)==0); }

    virtual bool isEof();

    virtual bool skipPosition(int32_t _Bytes) { return (seekCurrent(_Bytes)==0); }

protected:
    virtual bool _read(uint32_t uBytes, void* outBuffer);

    virtual bool _write(uint32_t uBytes, const void* inBuffer);

public:
    virtual uint32_t fread(uint32_t uMaxBytes, void* outBuffer);

    virtual uint32_t fwrite(uint32_t uMaxBytes, const void* inBuffer);

    virtual uint32_t fprintf(const char* pszFormat, ...);
};



