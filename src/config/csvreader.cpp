
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

    // ��Ԫ�����ݼ��ָ��
    std::string e, sign = ",\n\r";

    // �ڴ�����
    rows_     = cols_ = 0;
    data_.clear();
    data_.push_back(std::vector<std::string>());

    // ���ļ�
    if (!m_FileReader.open(filename))
    {
        m_FileReader.close();
        return false;
    }

    char dataChar;
    bool falg = false; // "�ű�־
    // �����ļ�����
    while (!m_FileReader.isEof())
    {
        m_FileReader.fread(1, &dataChar);
        // �Ƿ�""�ڵ�
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

        // �����ַ�
        if (sign.find_first_of(dataChar)==sign.npos)
        {
            e += dataChar;
        }
        else
        {
            // �жϵ�Ԫ������
            if (dataChar!='\r')
            {
                data_[rows_].push_back(e);
                e = "";
                // �жϻ���
                if (dataChar=='\n')
                {
                    data_.push_back(std::vector<std::string>());
                    rows_++;
                }
            }
        }
    }

    // �����ļ���ȡ��
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
    // �ж��ļ����ַ�����Ч��
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

// ����������ݵ�����
int CCsvConfigReader::GetRows()
{
    assert(backup_);
    return (rows_+1);
}

// ����������ݵ�����
int CCsvConfigReader::GetCols()
{
    assert(backup_);
    return cols_;
}

std::string CCsvConfigReader::ReadFieldValue(int row, int col)
{
    // �ж����ļ��Ƿ���Ч
    if (!instream_.is_open())
        return std::string("");

    std::string e, sign  = ",\n\r";
    char        ch;
    int         temp_row = 0;
    int         temp_col = 0;

    // ��¼ƫ��
    instream_.seekg(std::ios::beg);

    // ѭ�������ļ�����
    while (instream_.read((char*) &ch, 1))
    {
        // �����ַ�
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
            printf("���ݱ�:%s ������ȱ�٣���ǰ�ڣ�%d �� \n", filename_.c_str(), curcols_);
            return "";
        }

        if (curcols_>(int) data_[currows_].size())
            return std::string("");

        return data_[currows_][curcols_-1];
    }

    // �ж����ļ��Ƿ���Ч
    if (!instream_.is_open())
        return std::string("");

    std::string e, sign = ",\n\r";
    char        ch;

    instream_.seekg(seek_ofset_, std::ios::beg);

    // ѭ�������ļ�����
    while (instream_.read((char*) &ch, 1))
    {
        seek_ofset_++;
        // �����ַ�
        if (sign.find_first_of(ch)==sign.npos)
        {
            e += ch;
        }
        else
        {
            if (ch!='\r')
            {
                // ע��ѭ���ж�����
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

// ��ʼ��ѭ����־
void CCsvConfigReader::First(int row)
{
    if (backup_)
    {
        currows_ = row;
        curcols_ = 0;
        return;
    }

    // ��һ���ж�IsDone
    if (row==0)
    {
        seek_ofset_ = -1;
    }
    else
    {
        seek_ofset_ = 0;
    }

    // �ж����ļ��Ƿ���Ч
    if (!instream_.is_open())
        return;

    std::string e, sign = ",\n\r";
    char        ch;
    int         tmp_row = 0;

    //��¼ƫ��
    instream_.seekg(std::ios::beg);

    //ѭ�������ļ�����
    while ((tmp_row!=row) && instream_.read((char*) &ch, 1))
    {
        seek_ofset_++;

        //�����ַ�
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

// �ƶ�ѭ����־
void CCsvConfigReader::Next()
{
    if (backup_)
    {
        currows_++;
        curcols_ = 0;
        return;
    }

    //�ж����ļ��Ƿ���Ч
    if (!instream_.is_open())
        return;

    std::string e, sign = ",\n\r";
    char        ch;

    // ��¼ƫ��
    instream_.seekg(seek_ofset_, std::ios::beg);

    // ѭ�������ļ�����
    while (instream_.read((char*) &ch, 1))
    {
        seek_ofset_++;
        // �����ַ�
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

// �ж��Ƿ��β
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

    // �趨ƫ����
    instream_.seekg(++seek_ofset_, std::ios::beg);

    char ch;
    if (instream_.read((char*) &ch, 1))
    {
        return false;
    }

    return true;
}






