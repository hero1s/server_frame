
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "config/csvreader.h"

namespace {

}

CCsvConfigReader::CCsvConfigReader(void)
{
    backup_   = true;
    filename_ = "";
    rows_     = cols_ = 0;
    data_.push_back(std::vector<std::string>());
    seek_ofset_ = 0;
}

CCsvConfigReader::~CCsvConfigReader(void)
{
    instream_.close();
}

bool CCsvConfigReader::OpenFile(const char* filename, bool backup)
{
    backup_             = true;
    filename_           = filename;

    if (!backup_)
    {
        seek_ofset_ = 0;
        filename_   = filename;
        instream_.close();
        instream_.open(filename);

        return instream_.is_open();
    }

    // 单元格数据及分割符
    std::string e, sign = ",\n\r";

    // 内存清理
    rows_     = cols_ = 0;
    data_.clear();
    data_.push_back(std::vector<std::string>());

    // 打开文件
    if (!m_FileReader.open(filename))
    {
        m_FileReader.close();
        return false;
    }

    char dataChar;
    bool falg = false; // "号标志
    // 读出文件数据
    while (!m_FileReader.isEof())
    {
        m_FileReader.fread(1, &dataChar);
        // 是否""内的
        if (falg)
        {
            if (dataChar=='\r' || dataChar=='\n')
                continue;
            else
                e += dataChar;

            if (dataChar=='"')
                falg = false;

            continue;
        }
        if (dataChar=='"')
            falg = true;

        // 遍历字符
        if (sign.find_first_of(dataChar)==sign.npos)
        {
            e += dataChar;
        }
        else
        {
            // 判断单元格数据
            if (dataChar!='\r')
            {
                data_[rows_].push_back(e);
                e = "";
                // 判断换行
                if (dataChar=='\n')
                {
                    data_.push_back(std::vector<std::string>());
                    rows_++;
                }
            }
        }
    }

    // 销毁文件读取器
    m_FileReader.close();

    cols_ = data_[0].size();

    if ((int) data_[rows_].size()<cols_)
        rows_--;

    indexs_.clear();
    for (int i = 0; i<cols_; ++i)
    {
        if (indexs_.find(data_[0][i])!=indexs_.end())
        {
            assert(false);
            return false;
        }
        if (data_[0][i].length()<=0)
            break;

        indexs_.insert(std::map<std::string, int>::value_type(data_[0][i], i));
    }
    return true;
}

bool CCsvConfigReader::SaveFile(const char* fileName)
{
    // 判断文件名字符串有效性
    if (backup_==false || fileName==NULL || fileName[0]==0)
        return false;

    std::ofstream outcsv(fileName);

    for (int row = 0; row<rows_; row++)
    {
        for (int col = 0; col<cols_; col++)
        {
            outcsv << data_[row][col];
            if (col!=(cols_-1))
                outcsv << ",";
        }

        outcsv << std::endl;
    }

    outcsv.close();
    return true;
}

// 获得配置数据的行数
int CCsvConfigReader::GetRows()
{
    assert(backup_);
    return (rows_+1);
}

// 获得配置数据的列数
int CCsvConfigReader::GetCols()
{
    assert(backup_);
    return cols_;
}

std::string CCsvConfigReader::ReadFieldValue(int row, int col)
{
    // 判断流文件是否有效
    if (!instream_.is_open())
        return std::string("");

    std::string e, sign  = ",\n\r";
    char        ch;
    int         temp_row = 0;
    int         temp_col = 0;

    // 记录偏移
    instream_.seekg(std::ios::beg);

    // 循环读出文件数据
    while (instream_.read((char*) &ch, 1))
    {
        // 遍历字符
        if (sign.find_first_of(ch)==sign.npos)
        {
            e += ch;
        }
        else
        {
            if (ch!='\r')
            {
                temp_col++;

                if (ch=='\n')
                {
                    temp_row++;
                    temp_col = 0;
                }

                if (((temp_col-1)==col) && (temp_row==row))
                {
                    return e;
                }

                e = "";
            }
        }
    }

    return std::string("");
}

int CCsvConfigReader::GetRowCount()
{
    return rows_;
}

std::string CCsvConfigReader::ReadFieldValue()
{
    if (backup_)
    {
        curcols_++;
        if (curcols_>cols_)
        {
            printf("数据表:%s 配置项缺少，当前第：%d 列 \n", filename_.c_str(), curcols_);
            return "";
        }

        if (curcols_>(int) data_[currows_].size())
            return std::string("");

        return data_[currows_][curcols_-1];
    }

    // 判断流文件是否有效
    if (!instream_.is_open())
        return std::string("");

    std::string e, sign = ",\n\r";
    char        ch;

    instream_.seekg(seek_ofset_, std::ios::beg);

    // 循环读出文件数据
    while (instream_.read((char*) &ch, 1))
    {
        seek_ofset_++;
        // 遍历字符
        if (sign.find_first_of(ch)==sign.npos)
        {
            e += ch;
        }
        else
        {
            if (ch!='\r')
            {
                // 注意循环判断条件
                if (ch=='\n')
                {
                    seek_ofset_--;
                }
                return e;
            }
        }
    }

    return std::string("");
}

std::string CCsvConfigReader::GetFieldValue(int row, const std::string& strcol)
{
    std::map<std::string, int>::iterator it = indexs_.find(strcol);
    if (it==indexs_.end())
    {
        assert(false);
        return std::string("");
    }

    if (row>rows_)
    {
        assert(false);
        return std::string("");
    }

    if (it->second>=(int) data_[row].size())
    {
        assert(false);
        return std::string("");
    }

    return data_[row][it->second];
}

// 初始化循环标志
void CCsvConfigReader::First(int row)
{
    if (backup_)
    {
        currows_ = row;
        curcols_ = 0;
        return;
    }

    // 第一次判断IsDone
    if (row==0)
    {
        seek_ofset_ = -1;
    }
    else
    {
        seek_ofset_ = 0;
    }

    // 判断流文件是否有效
    if (!instream_.is_open())
        return;

    std::string e, sign = ",\n\r";
    char        ch;
    int         tmp_row = 0;

    //记录偏移
    instream_.seekg(std::ios::beg);

    //循环读出文件数据
    while ((tmp_row!=row) && instream_.read((char*) &ch, 1))
    {
        seek_ofset_++;

        //遍历字符
        if (sign.find_first_of(ch)==sign.npos)
        {
            e += ch;
        }
        else
        {
            if (ch!='\r')
            {
                if (ch=='\n')
                {
                    tmp_row++;
                }
                e = "";
            }
        }
    }
}

// 移动循环标志
void CCsvConfigReader::Next()
{
    if (backup_)
    {
        currows_++;
        curcols_ = 0;
        return;
    }

    //判断流文件是否有效
    if (!instream_.is_open())
        return;

    std::string e, sign = ",\n\r";
    char        ch;

    // 记录偏移
    instream_.seekg(seek_ofset_, std::ios::beg);

    // 循环读出文件数据
    while (instream_.read((char*) &ch, 1))
    {
        seek_ofset_++;
        // 遍历字符
        if (sign.find_first_of(ch)==sign.npos)
        {
            e += ch;
        }
        else
        {
            if (ch!='\r')
            {
                if (ch=='\n')
                {
                    return;
                }
                e = "";
            }
        }
    }

}

// 判断是否结尾
bool CCsvConfigReader::IsDone()
{
    if (backup_)
    {
        if ((rows_<currows_) || (cols_<=curcols_))
        {
            return true;
        }

        return false;
    }

    // 设定偏移量
    instream_.seekg(++seek_ofset_, std::ios::beg);

    char ch;
    if (instream_.read((char*) &ch, 1))
    {
        return false;
    }

    return true;
}






