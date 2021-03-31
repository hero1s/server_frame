
#pragma once

#ifndef _LARGEFILE_SOURCE
#define _LARGEFILE_SOURCE //for LFS support
#endif

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64 //for LFS support
#endif

#include <stdio.h>
#include <string>

#include "fundamental/common.h"
#include "string/stringutility.h"
#include <fcntl.h>
#include <map>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace svrlib {
class CFileHelper {
public:
    enum SEEK_TYPE {
        SK_BEG,
        SK_END,
        SK_CUR,
    };

    enum {
        MOD_RDONLY = 1,
        MOD_WRONLY_APPEND,
        MOD_WRONLY_TRUNC,
        MOD_RDWR_APPEND,
        MOD_RDWR_TRUNC,
    };
    enum {
        ERR_SIZE = -1,
    };

    CFileHelper(const std::string& pFileName, int iMod)
        : m_iFile(-1)
    {
        Open(pFileName, iMod);
    }

    ~CFileHelper()
    {
        Close();
    }

    std::string GetFileName() const
    {
        return m_strFileName;
    }

    void Flush()
    {
        if (m_iFile != -1) {

            sync();
        }
    }

    bool IsOpen() const
    {
        return m_iFile != -1;
    }

    bool Seek(uint64_t uiOff, int iPos = SK_BEG);

    bool Write(void const* pData, size_t uiLen);

    bool Write(uint64_t uiOff, void const* pData, size_t uiLen);

    /*
     功能：把一个map输出到一个文件中
     参数：
     mDict:Map参数
     strFileName:文件名
     strSeparator:分隔符
     返回值：是否成功
     */
    template <class datatype>
    bool DumpMap(const std::map<std::string, datatype>& mDict,
        const std::string strSeparator = "=")
    {
        for (typename std::map<std::string, datatype>::iterator it = mDict.begin();
             it != mDict.end();
             it++) {
            std::ostringstream oStr;
            oStr << it->first << strSeparator << it->second << "\r\n";

            if (!Write((const void*)(oStr.str().c_str()), oStr.str().size())) {
                return false;
            }
        }
        return true;
    }

    size_t Read(void* pData, size_t uiLen);

    bool ReadStrListFile(std::vector<std::string>& strList);

    /*
     功能：从文件加载一个map, 格式要求是每行都是<key = value>
     参数：
     mDict:返回值
     strFileName:文件名
     strSeparator:分隔符
     返回值：小于0失败，大于0返回个数
     */
    bool Load2Map(std::map<std::string, std::string>& mDict,
        const std::string& strSeparator);

    template <class datatype>
    bool Load2Map(std::map<std::string, datatype>& mDict,
        const std::string& strFileName,
        const std::string& strSeparator = "=")
    {
        // 首先调用上面的函数把数据导入到临时map中
        std::map<std::string, std::string> mTempDict;
        if (!Load2Map(mTempDict, strFileName, strSeparator)) {
            return false;
        }

        datatype Value;
        std::map<std::string, std::string>::iterator it;
        for (it = mTempDict.begin(); it != mTempDict.end(); it++) {
            Value = CStringUtility::ConvertFromStr<datatype>(it->second);
            mDict[it->first] = Value;
        }
        return true;
    }

    size_t Read(uint64_t uiOff, void* pData, size_t uiLen);

    bool ReadFixLen(uint64_t uiOff, void* pData, size_t uiLen);

    static bool IsExistFile(const std::string& pFileName)
    {
        return access(pFileName.c_str(), 0) == 0;
    }

    static bool DeleteFile(const std::string& pFileName)
    {
        return unlink(pFileName.c_str()) == 0;
    }

    uint64_t static GetFileSize(const std::string& pFileName)
    {
        struct stat64 buf;
        if (stat64(pFileName.c_str(), &buf) < 0) {
            return ERR_SIZE;
        }
        return buf.st_size;
    }

    bool Rename(const std::string& pOldName, const std::string& pNewName)
    {
        return 0 == rename(pOldName.c_str(), pNewName.c_str());
    }

    /** 功能：取得文件最后访问时间 */
    bool GetFileAccessTimeStr(const std::string& strFileName,
        std::string& strTime);

    /** 功能：取得文件创建时间 */
    bool GetFileCreateTimeStr(const std::string& strFileName,
        std::string& strTime);
    ;

    /** 功能：取得文件最后修改时间 */
    bool GetFileLastModifyTimeStr(const std::string& strFileName,
        std::string& strTime);

    /** 功能：取得文件最后访问时间 */
    bool GetFileAccessTime(const std::string& strFileName, time_t& stTime);

    /** 功能：取得文件创建时间 */
    bool GetFileCreateTime(const std::string& strFileName, time_t& stTime);

    /** 功能：取得文件最后修改时间 */
    bool GetFileLastModifyTime(const std::string& strFileName, time_t& stTime);

    /** 功能：从完整的文件路径名中取得文件名 */
    std::string ExtractFileName(const std::string& strFilePath);

    /** 功能：从文件名中取得扩展名 */
    bool ExtractFileExtName(const std::string& strFilePath,
        std::string& strFileExt);

    /*
     功能：根据路径创建相应的目录
     参数：
     pszFilePath：输入路径
     返回值：0为正确，失败为-1
     */
    static bool MkDirForFile(const char* pszFilePath);

    /////////////////////////////////////////////////////////////////
    // 下面是目录操作

    /*
     功能：判断目录是否存在
     参数：
     strDirName:目录路径
     返回值：是否存在
     */
    static bool IsDirExist(const std::string& strDirName);

    /*
     功能：一个字符串是不是一个目录名
     参数：
     strDirName:目录路径
     返回值：是否存在
     */
    static bool IsDir(const std::string& strDirName);

    /*
     功能：判断目录是否可读、写
     参数：
     strDirName:目录路径
     返回值：是否可读写
     */
    static bool IsDirReadable(const std::string& strDirName);

    static bool IsDirWritable(const std::string& strDirName);

    /*
     功能：创建一个目录
     参数：
     strDirName:目录路径
     返回值：0 创建成功，其他失败
     */
    static bool CreateDir(const std::string& strDirName);

    /*
     功能：设置当前目录
     参数：
     strDirName:目录路径
     返回值：0 创建成功，其他失败
     */
    static bool SetCurrentDir(const std::string& strDirName);

    static bool GetCurrentDir(std::string& strDirName);

    /*
     功能：获取一个目录下所有文件的路径
     bRecursion是否递归查找子目录，bSubDirsAdd输出结果中是否包括子目录的路径
     参数：
     strDirPath:目录路径
     filePaths:输出结果
     bRecursion:是否递归查找
     bSubDirsAdd:是否包含子mul
     返回值：小于0失败，大于0为文件个数
     */
    static bool DirFiles(const std::string& strDirPath,
        std::vector<std::string>& filePaths, bool bRecursion = true,
        bool bSubDirsAdd = false);

    void Close();

private:
    bool Open(const std::string& pFileName, int iMod);

    int m_iFile;
    std::string m_strFileName;
};
};
