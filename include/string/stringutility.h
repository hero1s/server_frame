#pragma once

#include <string>
#include <vector>
#include <set>
#include <list>
#include <sstream>
#include "fundamental/common.h"
#include <stdlib.h>

namespace svrlib {
/**
 * <p>string 的处理的utility</p>
 * <p>2008-10-18建立</p>
 * <p>20011-11 修改</p>
 * @author  peterzhao
 */
class CStringUtility {
public:

    /**
        转小写
        @param	strSrc
        @return 处理后源串的引用
    */

    static void ToLower(char* szSrc);

    static void ToLower(char* szSrc, size_t iLen);

    static std::string& GBKToLower(std::string& strSrc);

    static void GBKToLower(char* szSrc);

    /**
            转小写
            @param  szSrc 待处理串
            @nLen:串的长度
            @return 处理后源串的引用
        */
    static void GBKToLower(char* szSrc, size_t iLen);

    /**
        转大写
        @param	strSrc
        @return 处理后源串的引用
    */
    static void ToUpper(char* szSrc);

    /**
            转大写
            @param  szSrc 待处理串
            @nLen:串的长度
            @return 处理后源串的引用
        */
    static void ToUpper(char* szSrc, size_t iLen);

    static std::string& GBKToUpper(std::string& strSrc);

    static void GBKToUpper(char* szSrc);

    static void GBKToUpper(char* szSrc, size_t iLen);

    /**
        判读目标字符串是否以一个字符串开始

        @param strSrc:目标字符串
        @param strStart:开始字符串
        @return 是否包含

    */
    static bool StartWith(std::string& strSrc, const std::string& strStart);

    static bool StartWithNoCase(std::string& strSrc, const std::string& strStart);

    /**
        判读目标字符串是否以一个字符串结束
        @param strSrc 目标字符串
        @param strStart开始字符串
        @return 是否包含

    */
    static bool EndWith(std::string& strSrc, std::string& strEnd);

    static bool EndWithNoCase(std::string& strSrc, std::string& strEnd);

    /**
        判读两个字符串是否相等(忽略大小写)
        @param str1 源字符串
        @param str2目标串
        @return 是否相等

    */
    static bool EqualNoCase(std::string& str1, std::string& str2);

    /**
        功能:将\t\r\n转换成空格
        @param strSrc

    */
    static void RepaceControlCharWithSpace(std::string& strSrc);

    /**
        功能:将\t\r\n转换成空格
        @param szSrc 待处理串

    */
    static void RepaceControlCharWithSpace(char* strSrc);

    /**
        将\t\r\n转换成空格
        @param szSrc 待处理串
        @param nLen:串的长度
    */
    static void RepaceControlCharWithSpace(char* strSrc, size_t iLen);

    /**
        利用指定字符串去头去尾
        @param strSrc 待处理串
        @param pszTarget:需要去除的字符集
        @return 处理后的strSrc的引用
    */
    static std::string& Trim(std::string& sValue, const std::string& strTarget = " \t\r\n");

    static std::string& TrimLeft(std::string& sValue, const std::string& strTarget = " \t\r\n");

    static std::string& TrimRight(std::string& sValue, const std::string& strTarget = " \t\r\n");

    /**
        把字符串A中的子串B替换为C
        @param strSrc 待处理串,字符串A
        @param strNew:字符串C
        @param strOld:字符串B
        @return 处理后的strSrc的引用
    */
    static std::string& ReplaceStr(std::string& strSrc,
            const std::string& strNew,
            const std::string& strOld);

    /**
        把一个集合中的字符转换为对应的另一个集合中字符
                    如果strFromSet和strToSet的长度不同，则以短的为标准
        @param strSrc 待处理串
        @param strFromSet 被替换集合
        @param strToSet 替换集合
     */

    static void ReplaceCharSet(std::string& strSrc,
            const std::string& strFromSet,
            const std::string& strToSet);

    /**
        把一个字符串划分成多个字符串
        @param strSrc 待处理串
        @param strSpliter 分隔符
        @param vecStrs 结果串
        @param bReserveNullString 是否保留结束符
    */
    static void SplitString(const std::string& strSrc,
            const std::string strSpliter,
            std::vector<std::string>& vecStrs,
            bool bReserveNullString = false);

    // 从字符传中解析出数字队列
    // 字符的格式如 “3 | 5 | 6 ”，解析出数据队列 3,5,6
    template<typename TYPE>
    static void ParserNumberString(std::vector<TYPE>& RefNumVec, std::string& RefString, char chSeg,
            bool bFilterZero = false)
    {
        if (RefString.empty())
        {
            return;
        }

        short                  wLen    = 0;
        TYPE                   uValue  = 0;
        std::string            strSub;
        std::string::size_type wPos    = 0;
        std::string::size_type wPosEnd = 0;

        do
        {
            wPosEnd     = RefString.find(chSeg, wPos);
            if (wPosEnd==RefString.npos)
                wPosEnd = RefString.length();

            if (wPosEnd==wPos)
                continue;

            wLen   = wPosEnd-wPos;
            strSub = RefString.substr(wPos, wLen);

            if (!strSub.empty())
            {
                uValue = (TYPE) atol(strSub.c_str());
                if (!((TYPE) 0==uValue && bFilterZero))
                {
                    RefNumVec.push_back(uValue);
                }
            }
        }
        while ((wPos = wPosEnd+1)<RefString.length());
    }

    /**
     * 判断两个字符在不区分大小写的情况下是否相等
     */

    static bool CmpNoCase(char c1, char c2);

    /**
            在字符串中查找子串，不区分大小(向左、向右)
            @param strSrc 待处理串
            @param strSub 子串
            @param dwFrom 从第几个字符开始查找，按照查找的方向计算
            @return 匹配的位置
        */
    static std::string::size_type
    FindNoCase(const std::string& strSrc, const std::string& strSub, size_t dwFrom = 0);

    static std::string::size_type
    RFindNoCase(const std::string& strSrc, const std::string& strSub, size_t dwFrom = std::string::npos);

    /**
         在字符串中查找在中文字符串中开始查找一个宽字符串(向左、向右)
        @param strSrc pcSrc 待处理串
        @param strWChar pcWChar 子串
        @return 匹配的位置,没有匹配上返回NULL或者 std::string::npos
    */
    static const char* WFind(const char* pcSrc, const char* pcSub);

    static const char* RWFind(const char* pcSrc, const char* pcSub);

    static std::string::size_type WFind(const std::string& strSrc, const std::string& strSub);

    static std::string::size_type RWFind(const std::string& strSrc, const std::string& strSub);

    /*
        检测字符串中是否包含了某些子字符串中的一个
        @param strSrc  待处理串
        @param sSubStr 字串集合
        @return 是否包含
    */
    static bool IsSubStrContained(const std::string& strSrc, const std::vector<std::string>& sSubStr);

    /**
        检测字符串中是否包含了某些子字符串中的全部
        @param strSrc  待处理串
        @param sSubStr 字串集合
        @return 是否包含
    */
    static bool IsSubStrAllContained(const std::string& strSrc, const std::vector<std::string>& sSubStr);

    //移除字符串子串
    static void DeleteSubStr(std::string &str,const std::string &sub);

    /**
         获得一个字符串的(中文)字符个数, 中文等双字节字符只算1个
        @param strWord  待处理串
        @return 字符个数
    */
    static size_t GetCharCount(const std::string& strWord);

    static size_t GetChineseCharCount(std::string& strWord);

    /**
        获得一个字符串中某个字符的个数
        @param strWord 待处理串
        @param	c 目标字符
        @return 字符个数
    */
    static size_t GetCharCount(const std::string& strWord, char c);

    /**
         统计一个字符串中数字和字母的个数
        @param strWord  待处理串
        @param dwAlpha 返回字符个数
        @param dwDigit 返回数字个数
    */
    static void StatStr(const std::string& strWord, size_t& dwAlpha, size_t& dwDigit);

    /**
        功能: 获得一个字符串中某个子字符串的个数
        @param strSrc  待处理串
        @param strSubStr 字串
        @param bNoCase 是否大小写敏感
        @return 字符串个数
    */
    static size_t GetSubStrCnt(const std::string& strSrc, const std::string strSubStr, bool bNoCase = true);

    /**
              判断是否是空白(空格，中文全角空格)串
     */
    static bool IsWhiteSpaceStr(const std::string& strSrc, bool isContainsZH = false);

    /**
         判断一个字符串是否全是字母,  最后一个参数表示是否要把'_'算作字母
        @param strSrc  待处理串
        @return 是否是
    */
    static bool IsEnglishString(const std::string& strSrc, bool bLowLineAsAlpha = false);

    /**
        功能: 判断一个字符串是否全是数字组成
        @param strSrc  待处理串

    */
    static bool IsDigitString(const std::string& strSrc);

    /**
        功能: 计算GB2312汉字的序号,返回值是0到6767之间的数
        @param strSrc  待处理串
    */
    static int GetGB2312ChaInde(const unsigned char szStr[2]);

    /**
         判断一个字符串是否完全由GB2312的汉字组成
        @param strSrc  待处理串

    */
    static bool IsOnlyComposedOfGB2312Cha(const std::string& strSrc);

    /**
     *  判断字符是否是分界符 " \t\r\n-_[](){}:.,=*&^%$#@!~?<>/|'\""
     */
    static bool IsSeparator(char c);

    /**
     * 是否是16进制的一位整数
     */
    static bool IsHexChar(char c)
    {
        return (c>='a' && c<='f') || (c>='A' && c<='F') || (c>='0' && c<='9');
    }

    /*
       功能:清除html里面转义字符，比如&amp;等，直接清除掉
       @param strSrc 带转换字符串
       @return 转换成的目标
   */
    static std::string ClearHtmlMarkup(const std::string& strSrc);

    /*
        检查GB2312字符串有没有乱码，判断是否为全英文数字字符以及判断是否为双字节系统的字符
        @param szSrc szSrc待处理串
        @param bContainsNprint 是否处理不可打印字符
        @return是否有
    */
    //
    static bool CheckGbKString(const std::string& strSrc, bool bContainsNprint = true);

    static bool IsDBCS(const unsigned char szStr[2]);

    /**
        检查GB2312字符串中文个数
        @param strSrc 待处理串
        @return >0有，0没有，-1有乱码
    */
    static ssize_t GetGb2312ChineseCount(const std::string& strSrc);

    /**
     * 把Gb2312的字符串截断
     * @param strSrc 待处理串
     * @param uiLen 截断长度
     */
    static void TruncGb2312Str(std::string& strSrc, size_t uiLen);

    /**
        Url DeCoder
        @param strSrc 待处理串
        @return: 处理后的串
    */
    static std::string DecodeUrlStr(const std::string& strSrc);

    /**
            Url EnCoder
            @param strSrc 待处理串
            @return: 处理后的串
        */
    static std::string EncodeUrlStr(const std::string& strSrc);

    /**
        得到前缀和后缀之间的字符串

            @param strSrc 待处理串
            @param strSub得到的字符串
            @param strPrefix 前缀
            @param strSuffix 后缀
            @param iFromIndex 从第几个字符开始查找
            @param bNoCase是否大小写无关
            @return 如果在给定的字符串中无前缀或后缀 false,否则返回true
            处理后的串
    */
    static bool
    GetSubStrByPrefixSuffix(const std::string& strSrc, std::string& strSub, const std::string& strPrefix,
            const std::string& strSuffix, size_t iFromIndex = 0, bool bNoCase = true);

    /*
     功能:字符串和其他类型转换函数
     参数:转换目标
     返回值:转换成的目标
     */
    template<class datatype>
    static std::string ConvertToStr(datatype data)
    {
        std::ostringstream ostr;
        ostr << data;
        return ostr.str();
    }

    template<class datatype>
    static datatype ConvertFromStr(const std::string& strSrc)
    {
        if (strSrc.size()==0)
            return 0;
        datatype           value;
        std::istringstream istr(strSrc);
        istr >> value;
        if (istr.fail() || istr.bad())
            return 0;
        return value;
    }

    /**
        清除html标签
        @param strSrc: 待处理串
    */
    static void clear_html_tag(std::string& strSrc);

    /**
        功能:二进制buffer和hex串的相互转换
     @return: -1由于输出缓冲区空间不够而失败
     */

    static ssize_t
    BinBufToHexBuf(char* pHexBuf, size_t iHexBufSize, const unsigned char* pBinBuf, size_t iBinBufSize);

    static ssize_t
    BinBufToHexStr(char* pszHexStrBuf, size_t iHexStrBufSize, const unsigned char* pBinBuf, size_t iBinBufSize);

    static std::string BinBufToHexString(const uint8_t* pBuf, size_t uiLen);

    static ssize_t
    BinBufFromHexBuf(uint8_t* pBinBuf, size_t iBinBufSize, const char* pHexBuf, size_t iHexBufSize);

    static ssize_t BinBufFromHexStr(uint8_t* pBinBuf, size_t iBinBufSize, const char* pszHexStr);

    /**
        将数据以16进制格式输出到缓冲区中
        @return:输出到缓冲区的字符个数
    */
    static size_t GetMemHexInfo(char* pszOutBuf, size_t iOutBufSize, const void* pData, size_t iDataSize);

    /*
    功能:将数据以文件G、M、K的形式输出
    返回值:
        输出结果
  */
    static std::string GetFileSizeStr(uint64_t i_size);

    static std::string remove_space(const std::string& str);

    static std::string& toString(const std::vector<char>& v, std::string& buf);

    static void toVector(const char* date, const size_t nLen, std::vector<char>& RefVec);

    // 格式化成一个string
    static std::string FormatToString(const char* fmt, ...);

private:
    static int atoh(char c);

    static char htoa(int h);
};

};



