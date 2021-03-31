
#include "string/stringutility.h"
#include <algorithm>
#include <cstring>
#include <ctype.h>
#include <errno.h>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "string/string_functions.h"

namespace svrlib {
void CStringUtility::ToLower(char* szSrc)
{
    if (NULL == szSrc)
        return;

    for (int i = 0; szSrc[i] != '\0'; i++)
        szSrc[i] = tolower(szSrc[i]);
}

void CStringUtility::ToLower(char* szSrc, size_t iLen)
{
    if (NULL == szSrc)
        return;

    for (size_t i = 0; i < iLen; i++)
        szSrc[i] = tolower(szSrc[i]);
}

std::string& CStringUtility::GBKToLower(std::string& strSrc)
{
    for (size_t i = 0; i < strSrc.size(); i++) {
        if (strSrc[i] < 0)
            i++;
        else
            strSrc[i] = tolower(strSrc[i]);
    }

    return strSrc;
}

void CStringUtility::GBKToLower(char* szSrc)
{
    if (NULL == szSrc)
        return;

    for (int i = 0; szSrc[i] != '\0'; i++) {
        if (szSrc[i] < 0)
            i++;
        else
            szSrc[i] = tolower(szSrc[i]);
    }
}

void CStringUtility::GBKToLower(char* szSrc, size_t iLen)
{
    if (NULL == szSrc)
        return;

    for (size_t i = 0; i < iLen; i++) {
        if (szSrc[i] < 0)
            i++;
        else
            szSrc[i] = tolower(szSrc[i]);
    }
}

void CStringUtility::ToUpper(char* szSrc)
{
    if (NULL == szSrc)
        return;

    for (int i = 0; szSrc[i] != '\0'; i++)
        szSrc[i] = toupper(szSrc[i]);
}

void CStringUtility::ToUpper(char* szSrc, size_t nLen)
{
    if (NULL == szSrc)
        return;

    for (size_t i = 0; i < nLen; i++)
        szSrc[i] = tolower(szSrc[i]);
}

std::string& CStringUtility::GBKToUpper(std::string& strSrc)
{
    for (size_t i = 0; i < strSrc.size(); i++) {
        if (strSrc[i] < 0)
            i++;
        else
            strSrc[i] = toupper(strSrc[i]);
    }

    return strSrc;
}

void CStringUtility::GBKToUpper(char* szSrc)
{
    if (NULL == szSrc)
        return;

    for (int i = 0; szSrc[i] != '\0'; i++) {
        if (szSrc[i] < 0)
            i++;
        else
            szSrc[i] = toupper(szSrc[i]);
    }
}

void CStringUtility::GBKToUpper(char* szSrc, size_t iLen)
{
    if (NULL == szSrc)
        return;

    for (size_t i = 0; i < iLen; i++) {
        if (szSrc[i] < 0)
            i++;
        else
            szSrc[i] = toupper(szSrc[i]);
    }
}

bool CStringUtility::StartWith(std::string& strSrc, const std::string& strStart)
{
    size_t nLen = strStart.size();

    if (strSrc.size() >= nLen) {
        for (size_t i = 0; i < nLen; i++) {
            if (strSrc[i] != strStart[i])
                return false;
        }
        return true;
    } else
        return false;
}

bool CStringUtility::StartWithNoCase(std::string& strSrc, const std::string& strStart)
{
    size_t nLen = strStart.size();
    if (strSrc.length() >= nLen) {
        for (size_t i = 0; i < nLen; i++) {
            if (tolower(strSrc[i]) != tolower(strStart[i]))
                return false;
        }
        return true;
    } else
        return false;
}

bool CStringUtility::EndWith(std::string& strSrc, std::string& strEnd)
{
    size_t nLen = strEnd.size();
    size_t nSrcLen = strSrc.size();
    if (nSrcLen >= nLen) {
        for (size_t i = 0; i < nLen; i++) {
            if (strSrc[nSrcLen - nLen + i] != strEnd[i])
                return false;
        }

        return true;
    } else
        return false;
}

bool CStringUtility::EndWithNoCase(std::string& strSrc, std::string& strEnd)
{
    size_t nLen = strEnd.size();
    size_t nSrcLen = strSrc.size();
    if (nSrcLen >= nLen) {
        for (size_t i = 0; i < nLen; i++) {
            if (tolower(strSrc[nSrcLen - nLen + i]) != tolower(strEnd[i]))
                return false;
        }
        return true;
    } else
        return false;
}

bool CStringUtility::EqualNoCase(std::string& str1, std::string& str2)
{
    if (str1.size() != str2.size())
        return false;

    for (size_t i = 0; i < str1.length(); i++) {
        if (tolower(str1[i]) != tolower(str2[i]))
            return false;
    }

    return true;
}

void CStringUtility::RepaceControlCharWithSpace(std::string& strSrc)
{
    for (size_t i = 0; i < strSrc.size(); i++) {
        if (strSrc[i] == '\t' || strSrc[i] == '\r' || strSrc[i] == '\n')
            strSrc[i] = ' ';
    }
}

void CStringUtility::RepaceControlCharWithSpace(char* strSrc)
{
    size_t nLen = strlen(strSrc);
    for (size_t i = 0; i < nLen; i++) {
        if (strSrc[i] == '\t' || strSrc[i] == '\r' || strSrc[i] == '\n')
            strSrc[i] = ' ';
    }
}

void CStringUtility::RepaceControlCharWithSpace(char* strSrc, size_t iLen)
{
    for (size_t i = 0; i < iLen; i++) {
        if (strSrc[i] == '\t' || strSrc[i] == '\r' || strSrc[i] == '\n')
            strSrc[i] = ' ';
    }
}

bool CStringUtility::IsWhiteSpaceStr(const std::string& strSrc, bool isContainsZH)
{
    size_t nLen = strSrc.size();

    for (size_t i = 0; i < nLen; i++) {
        if (strSrc[i] != '\t' && strSrc[i] != '\r' && strSrc[i] != '\n' && strSrc[i] != ' ') {
            //这里判断中文全角字符里的那个空格，连续两个0xA1
            if (strSrc[i] < 0 && (i + 1) < nLen) {
                if (!isContainsZH) {
                    i++;
                    continue;
                }
                unsigned char c1 = (unsigned char)(strSrc[i]);
                unsigned char c2 = (unsigned char)(strSrc[i + 1]);
                if (c1 == 0xA1 && c2 == 0xA1)
                    i++;
                else
                    return false;
            } else
                return false;
        }
    }

    return true;
}

std::string& CStringUtility::Trim(std::string& sValue, const std::string& strTarget)
{
    return TrimRight(TrimLeft(sValue, strTarget), strTarget);
}

std::string& CStringUtility::TrimLeft(std::string& sValue, const std::string& strTarget)
{
    sValue.erase(0, sValue.find_first_not_of(strTarget));
    return sValue;
}

std::string& CStringUtility::TrimRight(std::string& sValue, const std::string& strTarget)
{
    sValue.erase(sValue.find_last_not_of(strTarget) + 1);
    return sValue;
}

/** 把字符串A中的子串B替换为C */
std::string& CStringUtility::ReplaceStr(std::string& strSrc,
    const std::string& strNew,
    const std::string& strOld)
{
    // 找到第一个Target
    size_t nPos = strSrc.find(strOld);

    while (std::string::npos != nPos) {
        if (strSrc.size() >= nPos + strOld.size()) {
            strSrc.erase(nPos, strOld.size());
            strSrc.insert(nPos, strNew);
        }

        nPos = strSrc.find(strOld, nPos + strNew.size());
    }

    return strSrc;
}

void CStringUtility::ReplaceCharSet(std::string& strSrc,
    const std::string& strFromSet,
    const std::string& strToSet)
{
    size_t iLen = 0;
    if (strFromSet.size() > strToSet.size())
        iLen = strFromSet.size();
    else
        iLen = strToSet.size();

    std::string strResult;

    for (size_t i = 0; i < strSrc.size(); i++) {
        char ch = strSrc[i];

        std::string::size_type dwPos = strFromSet.find(ch);
        if (iLen <= dwPos) {
            strResult += ch;
            continue;
        }

        if (dwPos == std::string::npos)
            strResult += ch;
        else if (dwPos < strToSet.size())
            strResult += strToSet[dwPos];
    }

    strSrc = strResult;
}

void CStringUtility::SplitString(const std::string& strSrc,
    const std::string strSpliter,
    std::vector<std::string>& vecStrs,
    bool bReserveNullString)
{
    vecStrs.clear();

    if (strSrc.empty() || strSpliter.empty())
        return;

    size_t nStartPos = 0;
    size_t nPos = strSrc.find(strSpliter);
    std::string strTemp;

    while (std::string::npos != nPos) {
        strTemp = strSrc.substr(nStartPos, nPos - nStartPos);
        if ((!strTemp.empty()) || (bReserveNullString && strTemp.empty()))
            vecStrs.push_back(strTemp);

        nStartPos = nPos + strSpliter.length();
        nPos = strSrc.find(strSpliter, nStartPos);
    }

    if (nStartPos != strSrc.size()) {
        strTemp = strSrc.substr(nStartPos);
        vecStrs.push_back(strTemp);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 字符串内容判断

bool CStringUtility::CmpNoCase(char c1, char c2)
{
    return toupper(c1) == toupper(c2);
}

/** 在字符串中查找子符串，不区分大小写 */
std::string::size_type
CStringUtility::FindNoCase(const std::string& strSrc, const std::string& strSub, size_t dwFrom)
{
    std::string::const_iterator it = std::search(strSrc.begin() + dwFrom, strSrc.end(), strSub.begin(),
        strSub.end(), CmpNoCase);

    if (it == strSrc.end())
        return std::string::npos;
    else
        return (it - strSrc.begin());
}

/** 在字符串中查找子符串，不区分大小写 */
std::string::size_type
CStringUtility::RFindNoCase(const std::string& strSrc, const std::string& strSub, size_t dwFrom)
{
    std::string::const_iterator it = std::find_end(strSrc.begin(), strSrc.begin() + dwFrom, strSub.begin(),
        strSub.end(), CmpNoCase);

    if (it == strSrc.end())
        return std::string::npos;
    else
        return (it - strSrc.begin());
}

/** 在中文字符串中查找宽字节的子符串 */
const char* CStringUtility::WFind(const char* pcSrc, const char* pcSub)
{
    if (NULL == pcSrc || NULL == pcSub)
        return NULL;

    if (!CheckGbKString(pcSrc) || !CheckGbKString(pcSub))
        return NULL;

    int nLen = strlen(pcSrc);
    int nSubLen = strlen(pcSub);
    if (nLen < nSubLen)
        return NULL;

    for (int i = 0; i < nLen; i++) {
        if (i + nSubLen > nLen)
            return NULL;

        bool bMatch = true;
        for (int j = 0; j < nSubLen; j++) {
            if (pcSrc[i + j] != pcSub[j]) {
                bMatch = false;
                break;
            }
        }
        if (bMatch)
            return &pcSrc[i];

        if (pcSrc[i] < 0)
            i++;
    }

    return NULL;
}

const char* CStringUtility::RWFind(const char* pcSrc, const char* pcSub)
{
    if (NULL == pcSrc || NULL == pcSub)
        return NULL;

    if (!CheckGbKString(pcSrc) || !CheckGbKString(pcSub))
        return NULL;

    const char* cRet = NULL;
    int nLen = strlen(pcSrc);
    int nSubLen = strlen(pcSub);
    if (nLen < nSubLen)
        return NULL;

    for (int i = 0; i < nLen; i++) {
        if (i + nSubLen > nLen)
            return cRet;

        bool bMatch = true;
        for (int j = 0; j < nSubLen; j++) {
            if (pcSrc[i + j] != pcSub[j]) {
                bMatch = false;
                break;
            }
        }
        if (bMatch)
            cRet = &pcSrc[i];

        if (pcSrc[i] < 0)
            i++;
    }

    return cRet;
}

std::string::size_type CStringUtility::WFind(const std::string& strSrc, const std::string& strSub)
{
    if (!CheckGbKString(strSrc) || !CheckGbKString(strSub))
        return std::string::npos;

    std::string::size_type nLen = strSrc.size();
    std::string::size_type nSubLen = strSub.size();

    if (nLen < nSubLen)
        return std::string::npos;
    if (nLen == 0 || nSubLen == 0)
        return std::string::npos;

    for (std::string::size_type i = 0; i < nLen; i++) {
        if (i + nSubLen > nLen)
            return std::string::npos;

        bool bMatch = true;
        for (std::string::size_type j = 0; j < nSubLen; j++) {
            if (strSrc[i + j] != strSub[j]) {
                bMatch = false;
                break;
            }
        }
        if (bMatch)
            return i;

        if (strSrc[i] < 0)
            i++;
    }

    return std::string::npos;
}

std::string::size_type CStringUtility::RWFind(const std::string& strSrc, const std::string& strSub)
{
    if (!CheckGbKString(strSrc) || !CheckGbKString(strSub))
        return std::string::npos;

    std::string::size_type nLen = strSrc.size();
    std::string::size_type nSubLen = strSub.size();
    std::string::size_type nRet = std::string::npos;

    if (nLen < nSubLen)
        return std::string::npos;
    if (nLen == 0 || nSubLen == 0)
        return std::string::npos;

    for (std::string::size_type i = 0; i < nLen; i++) {
        if (i + nSubLen > nLen)
            return nRet;

        bool bMatch = true;
        for (std::string::size_type j = 0; j < nSubLen; j++) {
            if (strSrc[i + j] != strSub[j]) {
                bMatch = false;
                break;
            }
        }
        if (bMatch)
            nRet = i;

        if (strSrc[i] < 0)
            i++;
    }

    return nRet;
}

/** 检测字符串中是否包含了某些子字符串中的一个 */
bool CStringUtility::IsSubStrContained(const std::string& strSrc, const std::vector<std::string>& sSubStr)
{
    std::vector<std::string>::const_iterator it;
    for (it = sSubStr.begin(); it != sSubStr.end(); it++) {
        if (std::string::npos != strSrc.find(*it))
            return true;
    }

    return false;
}

/** 检测字符串中是否包含了某些子字符串中的全部 */
bool CStringUtility::IsSubStrAllContained(const std::string& strSrc, const std::vector<std::string>& sSubStr)
{
    std::vector<std::string>::const_iterator it;
    for (it = sSubStr.begin(); it != sSubStr.end(); it++) {
        if (std::string::npos == strSrc.find(*it))
            return false;
    }

    return true;
}

//移除字符串子串
void CStringUtility::DeleteSubStr(std::string& str, const std::string& sub)
{
    int m, flag = 0, num = 0; //num是子串出现的次数
    while (flag == 0) {
        m = str.find(sub);
        if (m < 0)
            flag = 1;
        else {
            str.erase(m, sub.size()); //删除子串
            num++;
        }
    }
}

/** 获得一个字符串的字符个数, 中文等双字节字符只算1个 */
size_t CStringUtility::GetCharCount(const std::string& strWord)
{
    size_t dwCharCount = 0;
    for (size_t i = 0; i < strWord.size(); ++i, ++dwCharCount) {
        if (strWord[i] < 0)
            ++i;
    }
    return dwCharCount;
}

/** 获得一个字符串中中文字符的个数 */
size_t CStringUtility::GetChineseCharCount(std::string& strWord)
{
    size_t dwCharCount = 0;
    for (size_t i = 0; i < strWord.size();) {
        if (strWord[i] < 0) {
            i += 2;
            ++dwCharCount;
        } else
            ++i;
    }

    return dwCharCount;
}

/** 获得一个字符串中某个字符的个数 */
size_t CStringUtility::GetCharCount(const std::string& strWord, char c)
{
    size_t dwCharCount = 0;
    for (size_t i = 0; i < strWord.size(); ++i)
        if (strWord[i] == c)
            ++dwCharCount;

    return dwCharCount;
}

/** 统计一个字符串中数字和字符等个数 */
void CStringUtility::StatStr(const std::string& strWord, size_t& dwAlpha, size_t& dwDigit)
{
    dwAlpha = 0;
    dwDigit = 0;

    for (size_t i = 0; i < strWord.size(); ++i) {
        if ((strWord[i] >= 'a' && strWord[i] <= 'z') || (strWord[i] >= 'A' && strWord[i] <= 'Z'))
            dwAlpha++;
        else if (strWord[i] >= '0' && strWord[i] <= '9')
            dwDigit++;
    }
}

/** 获得一个字符串中某个子字符串的个数 */
size_t CStringUtility::GetSubStrCnt(const std::string& strSrc, const std::string strSubStr, bool bNoCase)
{
    size_t dwPos = 0;
    size_t dwSubStrCnt = 0;

    do {
        if (bNoCase)
            dwPos = CStringUtility::FindNoCase(strSrc, strSubStr, dwPos + strSubStr.size());
        else
            dwPos = strSrc.find(strSubStr, dwPos + strSubStr.size());
        if (std::string::npos != dwPos)
            dwSubStrCnt++;
        else
            break;
    } while (true);

    return dwSubStrCnt;
}

/** 判断一个字符串是否是完全由英文字符串组成 */
bool CStringUtility::IsEnglishString(const std::string& strSrc, bool bLowLineAsAlpha)
{
    for (size_t i = 0; i < strSrc.size(); i++) {
        if (bLowLineAsAlpha) {
            if (isalpha(strSrc[i]) == 0 && '_' != strSrc[i])
                return false;
        } else {
            if (isalpha(strSrc[i]) == 0)
                return false;
        }
    }
    return true;
}

/** 判断一个字符串是否全是数字组成 */
bool CStringUtility::IsDigitString(const std::string& strSrc)
{
    for (size_t i = 0; i < strSrc.size(); i++) {
        if (isdigit(strSrc[i]) == 0)
            return false;
    }
    return true;
}

/** 计算GB2312汉字的序号,返回值是0到6767之间的数 */
int CStringUtility::GetGB2312ChaInde(const unsigned char szStr[2])
{
    return ((unsigned char)szStr[0] - 176) * 94 + (unsigned char)szStr[1] - 161;
}

/** 判断一个字符串是否完全由GB2312的汉字组成 */
bool CStringUtility::IsOnlyComposedOfGB2312Cha(const std::string& strSrc)
{
    //如果字符串的长度不是2的倍数,则返回false
    if (strSrc.size() % 2 != 0)
        return false;

    //逐个判断每两个字节是否组成一个汉字
    for (size_t i = 0; i + 1 < strSrc.size(); i = i + 2) {
        unsigned char szTmp[2];
        szTmp[0] = strSrc[i];
        szTmp[1] = strSrc[i + 1];
        int iIndex = GetGB2312ChaInde(szTmp);
        if (iIndex < 0 || iIndex > 6767)
            return false;
    }
    return true;
}

/** 判断字符是否是分界符 */
bool CStringUtility::IsSeparator(char c)
{
    std::string strSeparator(" \t\r\n-_[](){}:.,=*&^%$#@!~?<>/|'\"");
    if (std::string::npos == strSeparator.find(c))
        return false;
    else
        return true;
}

//清除html里面转义字符，比如&amp;等，直接清除掉
std::string CStringUtility::ClearHtmlMarkup(const std::string& strSrc)
{
    std::string strDes;

    for (size_t index = 0; index < strSrc.size(); index++) {
        char c = strSrc[index];
        if ('&' == c) {
            int n = strSrc.find(';', index + 1);
            if (n > 0)
                index = n;
            else
                strDes.push_back(c);
        } else
            strDes.push_back(c);
    }
    return strDes;
}

bool CStringUtility::IsDBCS(const unsigned char szStr[2])
{
    return ((szStr[0] > 0xa0 && szStr[1] > 0xa0) //HZ_GB
        || (szStr[0] > 0xa0 && ((szStr[1] > 0x3f && szStr[1] < 0x7f) || (szStr[1] > 0xa0))) //HZ_BIG5
        || (((szStr[0] > 0x80 && szStr[0] < 0xa0) || (szStr[0] > 0xdf && szStr[0] < 0xfd)) && (szStr[1] > 0x3f && szStr[1] < 0xfd))
        //HZ_JPN
        || (szStr[0] > 0x80 && ((szStr[1] > 0x3f && szStr[1] < 0x7f) || (szStr[1] > 0x7f && szStr[1] < 0xff))) //HZ_GBK
        || ((szStr[0] > 0xaf && szStr[0] < 0xf8) && szStr[1] > 0xa0) //HZ_TGB
        || (((szStr[0] == 0xa1) || (szStr[0] == 0xa3))
            && szStr[1] > 0xa0) //HZ_TSIGN
        || (((szStr[0] == 0xa4) || (szStr[0] == 0xa5)) && (szStr[1] > 0xa0 && szStr[1] < 0xf4))); //HZ_TJPN
}

bool CStringUtility::CheckGbKString(const std::string& strSrc, bool bContainsNprint)
{
    size_t nLen = strSrc.size();

    for (size_t i = 0; i < nLen; i++) {
        if (strSrc[i] < 0) {
            if (i == nLen - 1)
                return false;
            unsigned char cTempChar[2];
            cTempChar[0] = (unsigned char)strSrc[i];
            cTempChar[1] = (unsigned char)strSrc[i + 1];
            if (IsDBCS(cTempChar)) {
                i++;
                continue;
            } else
                return false;
        } else if (!bContainsNprint && !isprint(strSrc[i]))
            return false;
    }

    return true;
}

ssize_t CStringUtility::GetGb2312ChineseCount(const std::string& strSrc)
{
    size_t nLen = strSrc.size();
    ssize_t iRet = 0;

    for (size_t i = 0; i < nLen; i++) {
        if (strSrc[i] < 0) {
            if (i == nLen - 1)
                return false;
            unsigned char cTempChar[2];
            cTempChar[0] = (unsigned char)strSrc[i];
            cTempChar[1] = (unsigned char)strSrc[i + 1];
            if (IsDBCS(cTempChar)) {
                iRet++;
                i++;
                continue;
            } else
                return -1;
        }
    }

    return iRet;
}

void CStringUtility::TruncGb2312Str(std::string& strSrc, size_t uiLen)
{
    size_t nSrcLen = strSrc.size();
    if (nSrcLen == 0)
        return;
    if (nSrcLen < uiLen)
        return;

    bool bTruned = false;
    for (size_t i = 0; i < uiLen; i++) {
        if (strSrc[i] < 0) {
            if (i == uiLen - 1) {
                bTruned = true;
                break;
            }

            unsigned char cTempChar[2];
            cTempChar[0] = (unsigned char)strSrc[i];
            cTempChar[1] = (unsigned char)strSrc[i + 1];
            if (IsDBCS(cTempChar)) {
                i++;
                continue;
            } else {
                bTruned = true;
                break;
            }
        }
    }

    if (bTruned) {
        strSrc.erase(uiLen - 1);
    } else
        strSrc.erase(uiLen);
}

//把%CB%C0%C9%F1%C0%B4%C1%CB的解码成“死神来了”
std::string CStringUtility::DecodeUrlStr(const std::string& strSrc)
{
    std::string strDes;

    for (size_t i = 0; i < strSrc.size(); i++) {
        if (strSrc[i] == '%' && (i + 2) < strSrc.length() && IsHexChar(strSrc[i + 1])
            && IsHexChar(strSrc[i + 2])) {
            char cTempChar[3];
            cTempChar[2] = 0;
            cTempChar[0] = strSrc[i + 1];
            cTempChar[1] = strSrc[i + 2];
            char c = (char)strtoul(cTempChar, 0, 16);
            strDes.push_back(c);
            i += 2;
        } else
            strDes.push_back(strSrc[i]);
    }
    return strDes;
}

std::string CStringUtility::EncodeUrlStr(const std::string& strSrc)
{
    std::string strDes;

    for (size_t i = 0; i < strSrc.size(); i++) {
        if ((strSrc[i] >= '0' && strSrc[i] <= '9') || (strSrc[i] >= 'a' && strSrc[i] <= 'z') || (strSrc[i] >= 'A' && strSrc[i] <= 'Z') || strSrc[i] == '.') {
            strDes.push_back(strSrc[i]);
        } else {
            char cTempChar[3];
            sprintf(cTempChar, "%02X", strSrc[i]);
            strDes.push_back('%');
            strDes.append(cTempChar);
        }
    }

    return strDes;
}

bool CStringUtility::GetSubStrByPrefixSuffix(const std::string& strSrc, std::string& strSub,
    const std::string& strPrefix, const std::string& strSuffix,
    size_t iFromIndex, bool bNoCase)
{
    std::string::size_type iPrefixIndex = std::string::npos;

    if (bNoCase) {
        iPrefixIndex = FindNoCase(strSrc, strPrefix, iFromIndex);
    } else {
        iPrefixIndex = strSrc.find(strPrefix, iFromIndex);
    }

    if (iPrefixIndex == std::string::npos)
        return false;

    std::string::size_type iSubStrIndex = iPrefixIndex + strPrefix.length();
    std::string::size_type iSuffixIndex = std::string::npos;

    if (bNoCase) {
        iSuffixIndex = FindNoCase(strSrc, strSuffix, iSubStrIndex);
    } else {
        iSuffixIndex = strSrc.find(strSuffix, iSubStrIndex);
    }

    if (iSuffixIndex == std::string::npos)
        return false;

    strSub = strSrc.substr(iSubStrIndex, iSuffixIndex - iSubStrIndex);

    return true;
}

//清除html标签
void CStringUtility::clear_html_tag(std::string& strSrc)
{
    const int STATUS_TEXT = 0;
    const int STATUS_IN_HTML = 1;

    int flag = STATUS_TEXT;
    std::string strResult = "";
    for (size_t i = 0; i < strSrc.size(); i++) {
        char c = strSrc[i];

        switch (flag) {
        case STATUS_TEXT:
            if (c == '<')
                flag = STATUS_IN_HTML;
            else
                strResult.push_back(c);
            break;

        case STATUS_IN_HTML:
            if (c == '>')
                flag = STATUS_TEXT;
            break;
        default:

            strResult.push_back(c);
            break;
        }
    }

    strSrc = strResult;
}

char CStringUtility::htoa(int h)
{
    if (h >= 0 && h <= 9)
        return static_cast<char>(h + '0');
    else if (h >= 10 && h <= 15)
        return static_cast<char>(h - 10 + 'a');
    else
        return '-';
}

int CStringUtility::atoh(char c)
{
    if (c >= '0' && c <= '9') {
        return static_cast<int>(c - '0');
    } else if (c >= 'a' && c <= 'f') {
        return static_cast<int>(c - 'a' + 10);
    } else if (c >= 'A' && c <= 'F') {
        return static_cast<int>(c - 'A' + 10);
    } else {
        return 0;
    }
}

ssize_t CStringUtility::BinBufToHexBuf(char* pHexBuf, size_t iHexBufSize, const unsigned char* pBinBuf,
    size_t iBinBufSize)
{
    if (iHexBufSize < (iBinBufSize << 1)) {
        // dst buffer space is not enough
        return -1;
    }

    ssize_t iRet = 0;

    for (size_t i = 0; i < iBinBufSize; ++i) {
        pHexBuf[iRet++] = toupper(htoa((pBinBuf[i] >> 4) & 0x0f));
        pHexBuf[iRet++] = toupper(htoa(pBinBuf[i] & 0x0f));
    }

    return iRet;
}

ssize_t CStringUtility::BinBufToHexStr(char* pszHexStrBuf, size_t iHexStrBufSize, const unsigned char* pBinBuf,
    size_t iBinBufSize)
{
    if (iHexStrBufSize < ((iBinBufSize << 1) + 1)) {
        // dst buffer space is not enough
        return -1;
    }

    ssize_t iRet = BinBufToHexBuf(pszHexStrBuf, iHexStrBufSize - 1, pBinBuf, iBinBufSize);

    pszHexStrBuf[iRet] = '\0'; // zero end

    return iRet;
}

std::string CStringUtility::BinBufToHexString(const uint8_t* pBuf,
    size_t uiLen)
{

    const size_t MAX_OSS_LEN = 1024 * 2;

    if (uiLen > MAX_OSS_LEN) {
        std::ostringstream oss;
        for (size_t i = 0; i < uiLen; ++i) {
            oss << std::setw(2) << std::hex << std::setfill('0')
                << (unsigned short)pBuf[i];
        }

        return oss.str();
    } else {
        char* p = new char[uiLen * 2 + 1];
        p[uiLen * 2] = 0;
        std::ostringstream oss;
        std::string str;
        for (unsigned int i = 0; i < uiLen; ++i) {
            oss.str("");
            oss << std::setw(2) << std::hex << std::setfill('0')
                << (unsigned short)pBuf[i];
            str = oss.str();
            p[i * 2] = str[0];
            p[i * 2 + 1] = str[1];
        }
        str = p;
        delete[] p;
        return str;
    }
}

ssize_t CStringUtility::BinBufFromHexBuf(unsigned char* pBinBuf, size_t iBinBufSize, const char* pHexBuf,
    size_t iHexBufSize)
{
    if (iHexBufSize > (iBinBufSize << 1)) {
        // dst buffer space is not enough
        return -1;
    }

    ssize_t iRet = 0;

    size_t iTotal = iHexBufSize >> 1;

    for (size_t i = 0; i < iTotal; ++i) {
        pBinBuf[iRet++] = (atoh(pHexBuf[i << 1]) << 4) | atoh(pHexBuf[(i << 1) + 1]);
    }

    return iRet;
}

ssize_t CStringUtility::BinBufFromHexStr(unsigned char* pBinBuf, size_t iBinBufSize, const char* pszHexStr)
{
    return BinBufFromHexBuf(pBinBuf, iBinBufSize, pszHexStr, strlen(pszHexStr));
}

size_t CStringUtility::GetMemHexInfo(char* pszOutBuf, size_t iOutBufSize, const void* pData, size_t iDataSize)
{
    size_t iPos = 0;

    // title
    iPos += snprintf(pszOutBuf + iPos, iOutBufSize - 1 - iPos,
        "\n----------  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f   ----------------");

    // content
    size_t iLine = (iDataSize + 15) >> 4;
    for (size_t i = 0; i < iLine; ++i) {
        const unsigned char* pBegin = reinterpret_cast<const unsigned char*>(pData) + (i << 4);

        // address
        iPos += snprintf(pszOutBuf + iPos, iOutBufSize - 1 - iPos, "\n0x%p  ", pBegin);

        // hex
        if (i < (iLine - 1) || (iDataSize & 0xf) == 0) {
            for (size_t j = 0; j < 16; ++j) {

                iPos += snprintf(pszOutBuf + iPos, iOutBufSize - 1 - iPos, "%02x ", pBegin[j]);
            }
        } else {
            // last line, un-integrated
            size_t iLeft = iDataSize & 0xf;
            for (size_t j = 0; j < iLeft; ++j) {
                iPos += snprintf(pszOutBuf + iPos, iOutBufSize - 1 - iPos, "%02x ", pBegin[j]);
            }

            // fill with spaces
            for (size_t j = iLeft; j < 16; ++j) {

                iPos += snprintf(pszOutBuf + iPos, iOutBufSize - 1 - iPos, "   ");
            }
        }

        iPos += snprintf(pszOutBuf + iPos, iOutBufSize - 1 - iPos, " ");

        // value
        if (i < (iLine - 1) || (iDataSize & 0xf) == 0) {
            for (size_t j = 0; j < 16; ++j) {

                iPos += snprintf(pszOutBuf + iPos, iOutBufSize - 1 - iPos, "%c",
                    isprint(pBegin[j]) > 0 ? pBegin[j] : '.');
            }
        } else {
            // last line, un-integrated
            size_t iLeft = iDataSize & 0xf;

            for (size_t j = 0; j < iLeft; ++j) {
                iPos += snprintf(pszOutBuf + iPos, iOutBufSize - 1 - iPos,
                    "%c", isprint(pBegin[j]) > 0 ? pBegin[j] : '.');
            }
        }

        // space of out buf is not enough
        if (iPos == (iOutBufSize - 1))
            break;
    }
    iPos += snprintf(pszOutBuf + iPos, iOutBufSize - 1 - iPos, "\n");

    return iPos;
}

void print_size(char* s_size, double dSize, const char* s)
{
    if (dSize >= 100)
        sprintf(s_size, "%3.0f%s", dSize, s);
    else if (dSize >= 10)
        sprintf(s_size, "%3.1f%s", dSize, s);
    else
        sprintf(s_size, "%3.2f%s", dSize, s);
}

std::string CStringUtility::GetFileSizeStr(uint64_t i_size)
{
    char s_size[1024];
    double dSize = (double)i_size;
    if (dSize < 999.5) {
        sprintf(s_size, "%3.0fB", dSize);
        return s_size;
    }
    dSize /= 1024;
    if (dSize < 999.5) {
        print_size(s_size, dSize, "K");
        return s_size;
    }
    dSize /= 1024;
    if (dSize < 999.5) {
        print_size(s_size, dSize, "M");
        return s_size;
    }

    dSize /= 1024;
    if (dSize < 999.5) {
        print_size(s_size, dSize, "G");
        return s_size;
    }
    s_size[0] = '\0';
    return s_size;
}

std::string CStringUtility::remove_space(const std::string& str)
{
    std::string tmp;
    std::remove_copy_if(str.begin(), str.end(), back_inserter(tmp), static_cast<int (*)(int)>(isspace));
    return tmp;
}

std::string& CStringUtility::toString(const std::vector<char>& v, std::string& buf)
{
    buf.clear();
    std::copy(v.begin(), v.end(), back_inserter(buf));
    return buf;
}

void CStringUtility::toVector(const char* date, const size_t nLen, std::vector<char>& RefVec)
{
    RefVec.resize(nLen);
    RefVec.clear();
    for (size_t i = 0; i < nLen; ++i) {
        RefVec[i] = date[i];
    }
}

// 格式化成一个string
std::string CStringUtility::FormatToString(const char* fmt, ...)
{
    char buffer[1024];
    unique_ptr<char[]> release1;
    char* base;
    for (int iter = 0; iter < 2; iter++) {
        int bufsize;
        if (iter == 0) {
            bufsize = sizeof(buffer);
            base = buffer;
        } else {
            bufsize = 30000;
            base = new char[bufsize];
            release1.reset(base);
        }
        char* p = base;
        char* limit = base + bufsize;
        if (p < limit) {
            va_list ap;
            va_start(ap, fmt);
            p += vsnprintf(p, limit - p, fmt, ap);
            va_end(ap);
        }
        // Truncate to available space if necessary
        if (p >= limit) {
            if (iter == 0) {
                continue; // Try again with larger buffer
            } else {
                p = limit - 1;
                *p = '\0';
            }
        }
        break;
    }
    return base;
}

};
