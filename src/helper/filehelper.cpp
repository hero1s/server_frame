/*
 * filehelper.cpp
 *
 *  Created on: 2011-11-6
 *      Author: pizhou
 */
#include "file/filehelper.h"
#include <cstdarg>
#include <cstring>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/resource.h>

#define MAX_PATH_LEN 1024

using namespace svrlib;

using std::istringstream;

bool CFileHelper::Seek(uint64_t uiOff, int iPos)
{
    if (m_iFile == -1) {
        return false;
    }
    if (iPos == SK_BEG) {
        iPos = SEEK_SET;
    } else if (iPos == SK_END) {
        iPos = SEEK_END;
    } else if (iPos == SK_CUR) {
        iPos = SEEK_CUR;
    } else {
        return false;
    }

    return lseek(m_iFile, uiOff, iPos) != -1;
}

bool CFileHelper::Write(void const* pData, size_t uiLen)
{
    if (m_iFile != -1) {

        return write(m_iFile, pData, uiLen) == (ssize_t)(uiLen);
    }
    return false;
}

bool CFileHelper::Write(uint64_t uiOff, void const* pData, size_t uiLen)
{
    if (!Seek(uiOff)) {
        return false;
    }
    return Write(pData, uiLen);
}

size_t CFileHelper::Read(void* pData, size_t uiLen)
{
    if (m_iFile != -1) {
        size_t uiHaveRead = 0;
        while (uiHaveRead < uiLen) {
            ssize_t iRead = read(m_iFile, (unsigned char*)pData + uiHaveRead,
                uiLen - uiHaveRead);

            if (iRead <= 0) {
                break;
            } else {
                uiHaveRead += iRead;
            }
        }
        return uiHaveRead;
    }
    return 0;
}

bool CFileHelper::ReadStrListFile(std::vector<std::string>& strList)
{

    size_t fileLength = GetFileSize(m_strFileName);

    char* buffer = new char[fileLength + 1];
    if (buffer == NULL) {
        return false;
    }

    bool bReadSuccess = Read(buffer, fileLength);
    if (bReadSuccess) {
        buffer[fileLength] = '\0';
        istringstream fs(buffer);
        std::string strLine;
        while (getline(fs, strLine)) {
            CStringUtility::Trim(strLine);
            if (!strLine.empty()) {
                strList.push_back(strLine);
            }
        }
    }

    delete[] buffer;

    return bReadSuccess;
}

/*
 功能：从文件加载一个map, 格式要求是每行都是<key = value>
 参数：
 mDict:返回值
 strFileName:文件名
 strSeparator:分隔符
 返回值：小于0失败，大于0返回个数
 */
bool CFileHelper::Load2Map(std::map<std::string, std::string>& mDict,
    const std::string& strSeparator)
{
    std::vector<std::string> tempString;
    if (!ReadStrListFile(tempString)) {
        return false;
    }

    std::string strLine;
    std::string strKey;
    std::string strValue;
    size_t dwPos;

    for (std::vector<std::string>::size_type i = 0; i < tempString.size(); ++i) {
        strLine = tempString[i];
        dwPos = strLine.find(strSeparator);
        if (std::string::npos != dwPos) {
            strKey = strLine.substr(0, dwPos);
            strKey = CStringUtility::Trim(strKey, " \t\r\n");
            if (strKey.empty())
                continue;

            strValue = strLine.substr(dwPos + strSeparator.size());
            strValue = CStringUtility::Trim(strValue, " \t\r\n");
            if (strValue.empty())
                continue;

            mDict[strKey] = strValue;
        }
    }

    return true;
}

size_t CFileHelper::Read(uint64_t uiOff, void* pData, size_t uiLen)
{
    if (!Seek(uiOff)) {
        return 0;
    }
    return Read(pData, uiLen);
}

bool CFileHelper::ReadFixLen(uint64_t uiOff, void* pData, size_t uiLen)
{
    if (!Seek(uiOff)) {
        return false;
    }
    size_t uiReadLen = 0;
    while (uiLen > 0) {
        uiReadLen = Read((uint8_t*)pData + uiReadLen, uiLen);
        if (uiReadLen == 0) {
            return false;
        }
        uiLen -= uiReadLen;
    }
    return true;
}

/** 功能：取得文件最后访问时间 */
bool CFileHelper::GetFileAccessTimeStr(const std::string& strFileName,
    std::string& strTime)
{
    struct stat stFileStat;
    int iRet = stat(strFileName.c_str(), &stFileStat);
    if (iRet < 0)
        return false;
    else
        strTime = ctime(&stFileStat.st_atime);
    return true;
}

/** 功能：取得文件创建时间 */
bool CFileHelper::GetFileCreateTimeStr(const std::string& strFileName,
    std::string& strTime)
{
    struct stat stFileStat;
    int iRet = stat(strFileName.c_str(), &stFileStat);
    if (iRet < 0)
        return false;
    else
        strTime = ctime(&stFileStat.st_ctime);
    return true;
}

/** 功能：取得文件最后修改时间 */
bool CFileHelper::GetFileLastModifyTimeStr(const std::string& strFileName,
    std::string& strTime)
{
    struct stat stFileStat;
    int iRet = stat(strFileName.c_str(), &stFileStat);
    if (iRet < 0)
        return false;
    else
        strTime = ctime(&stFileStat.st_mtime);
    return true;
}

/** 功能：取得文件最后访问时间 */
bool CFileHelper::GetFileAccessTime(const std::string& strFileName,
    time_t& stTime)
{
    struct stat stFileStat;
    int iRet = stat(strFileName.c_str(), &stFileStat);
    if (iRet < 0)
        return false;
    else
        stTime = stFileStat.st_atime;
    return true;
}

/** 功能：取得文件创建时间 */
bool CFileHelper::GetFileCreateTime(const std::string& strFileName,
    time_t& stTime)
{
    struct stat stFileStat;
    int iRet = stat(strFileName.c_str(), &stFileStat);
    if (iRet < 0)
        return false;
    else
        stTime = stFileStat.st_ctime;
    return true;
}

/** 功能：取得文件最后修改时间 */
bool CFileHelper::GetFileLastModifyTime(const std::string& strFileName,
    time_t& stTime)
{
    struct stat stFileStat;
    int iRet = stat(strFileName.c_str(), &stFileStat);
    if (iRet < 0)
        return false;
    else
        stTime = stFileStat.st_mtime;
    return true;
}

/** 功能：从完整的文件路径名中取得文件名 */
std::string CFileHelper::ExtractFileName(const std::string& strFilePath)
{
    const char* PATH_SEPARATOR_STR = "/";
    size_t iPos = strFilePath.rfind(PATH_SEPARATOR_STR);

    if (iPos == std::string::npos)
        return strFilePath;
    else
        return strFilePath.substr(iPos + 1, strFilePath.size() - iPos - 1);
}

/** 功能：从文件名中取得扩展名 */
bool CFileHelper::ExtractFileExtName(const std::string& strFilePath,
    std::string& strFileExt)
{
    size_t iPos = strFilePath.rfind('.');
    if (iPos == std::string::npos)
        return false;

    strFileExt = strFilePath.substr(iPos + 1, strFilePath.size() - iPos - 1);
    return true;
}

bool CFileHelper::Open(const std::string& pFileName, int iMod)
{
    m_strFileName = pFileName;
    if (m_strFileName.empty()) {
        return false;
    }
    int iOpenMod = 0;

    switch (iMod) {
    case MOD_RDONLY:
        iOpenMod = O_RDONLY;
        break;

    case MOD_WRONLY_APPEND:
        iOpenMod = O_CREAT | O_APPEND | O_WRONLY;
        break;

    case MOD_WRONLY_TRUNC:
        iOpenMod = O_CREAT | O_TRUNC | O_WRONLY;
        break;

    case MOD_RDWR_APPEND:
        iOpenMod = O_CREAT | O_APPEND | O_RDWR;
        break;

    case MOD_RDWR_TRUNC:
        iOpenMod = O_CREAT | O_TRUNC | O_RDWR;
        break;
    };
    m_iFile = open(m_strFileName.c_str(), iOpenMod, S_IREAD | S_IWRITE);

    return m_iFile != -1;
}

void CFileHelper::Close()
{
    Flush();
    if (m_iFile != -1) {
        close(m_iFile);
        m_iFile = -1;
    }
}

bool CFileHelper::MkDirForFile(const char* pszFilePath)
{
    if (pszFilePath != NULL) {
        char szDir[MAX_PATH_LEN];
        const char* pBegin = pszFilePath;

        while (*pBegin != '\0') {
            const char* p = strstr(pBegin, "/");
            if (p != NULL) {
                int iDirLen = static_cast<int>(p - pszFilePath);
                memcpy(szDir, pszFilePath, iDirLen);
                szDir[iDirLen] = 0;
                if (CreateDir(szDir) == false)
                    return false;
                pBegin = p + 1;
            } else
                break;
        }
    }
    return true;
}

/** 功能：判断目录是否存在 */
bool CFileHelper::IsDirExist(const std::string& strDirName)
{
    return IsDir(strDirName);
}

/** 功能：一个字符串是不是一个目录名 */
bool CFileHelper::IsDir(const std::string& strDirName)
{
    struct stat buf;
    if (!(stat(strDirName.c_str(), &buf) == 0))
        return false;
    return (buf.st_mode & S_IFDIR) != 0;
}

/** 功能：判断目录是否可读 */
bool CFileHelper::IsDirReadable(const std::string& strDirName)
{
    if (!IsDirExist(strDirName))
        return false;
    return access(strDirName.c_str(), R_OK) == 0;
}

/** 功能：判断目录是否可写 */
bool CFileHelper::IsDirWritable(const std::string& strDirName)
{
    if (!IsDirExist(strDirName))
        return false;
    return access(strDirName.c_str(), W_OK) == 0;
}

/** 功能：创建一个目录 */
bool CFileHelper::CreateDir(const std::string& strDirName)
{
    return (mkdir(strDirName.c_str(), 0777) == 0);
}

/** 功能：设置当前目录 */
bool CFileHelper::SetCurrentDir(const std::string& strDirName)
{
    // 首先检查目录是否存在
    if (!IsDirExist(strDirName))
        return false;
    if (chdir(strDirName.c_str()) == 0)
        return true;
    return false;
}

/** 功能：获得当前目录 */
bool CFileHelper::GetCurrentDir(std::string& strDirName)
{

    char szPath[MAX_PATH_LEN];
    if (getcwd(szPath, MAX_PATH_LEN) == NULL)
        return false;
    strDirName = szPath;

    return true;
}

//把sDirPath目录下的所有文件的路径获取
bool CFileHelper::DirFiles(const std::string& strDirPath,
    std::vector<std::string>& filePaths, bool bRecursion /*= true*/,
    bool bSubDirsAdd /*= false*/)
{
    filePaths.clear();

    std::string strTempPath = strDirPath;
    if (strTempPath.empty())
        return 0;
    if (strTempPath[strTempPath.size() - 1] != '/')
        strTempPath += "/";

    DIR* dp;
    if ((dp = opendir(strTempPath.c_str())) == NULL)
        return false;
    struct dirent* dirp;
    while ((dirp = readdir(dp)) != NULL) {
        struct dirent temp = (*dirp);
        if (0 == strcmp(temp.d_name, ".") || 0 == strcmp(temp.d_name, ".."))
            continue;

        std::string strTempSubdirPath = strTempPath + temp.d_name;
        if (IsDir(strTempSubdirPath)) {
            if (bRecursion)
                DirFiles(strTempSubdirPath, filePaths, bRecursion, bSubDirsAdd);
            if (bSubDirsAdd)
                filePaths.push_back(strTempSubdirPath);
        } else
            filePaths.push_back(strTempSubdirPath);
    }

    closedir(dp);
    return true;
}
