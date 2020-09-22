
#pragma once

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include "helper/fileStream.h"
#include <sstream>

class CCsvConfigReader {
public:
    CCsvConfigReader(void);

    ~CCsvConfigReader(void);

public:
    // 打开文件
    bool OpenFile(const char* filename, bool backup = true);

    // 保存文件
    bool SaveFile(const char* filename);

    // 获得配置数据的行数
    int GetRows();

    // 获得配置数据的列数
    int GetCols();

public:
    // 初始化循环标志
    void First(int nRow = 0);

    // 移动循环标志
    void Next();

    // 判断是否结尾
    bool IsDone();

public:
    // 返回字符串字段
    template<typename T>
    T GetFieldValue()
    {
        T            asVal;
        stringstream ss;
        ss << this->ReadFieldValue();
        ss >> asVal;
        return asVal;
    }

    // 根据返回字符串字段
    template<typename T>
    T GetFieldValue(int rownum, std::string strcol)
    {
        T            asVal;
        stringstream ss;
        ss << this->GetFieldValue(rownum, strcol);
        ss >> asVal;
        return asVal;
    }

    int GetRowCount();

protected:
    //直接从文件流读取值
    std::string ReadFieldValue(int row, int col);

    //循环遍历读取值
    std::string ReadFieldValue();

    std::string GetFieldValue(int row, const std::string& strcol);

private:
    CFileStream   m_FileReader;            // 文件读取器
    bool          backup_;                // 内存备份开关
    std::ifstream instream_;                // 文件输入流对象
    std::string   filename_;                // 文件名
    int           seek_ofset_;            // 当前文件字节偏移量

    int currows_;                // 当前内存数据行
    int curcols_;                // 当前内存数据列

private:
    std::vector<std::vector<std::string> > data_;                    // 内存数据
    int                                    rows_;                    // 内存数据行
    int                                    cols_;                    // 内存数据列
    std::map<std::string, int>             indexs_;                // 索引信息

};





