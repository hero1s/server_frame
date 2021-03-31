
#pragma once

#include "utility/basic_types.h"
#include "utility/basic_functions.h"
#include "string/string_functions.h"
#include <string>
#include "dbmysql/dbmysql.h"
#include "dbmysql/sql_join.h"
#include <vector>

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <iostream>
#include <sstream>
#include <map>

using namespace std;

class MYSQLValue {
public:
    enum {
      emTYPE_STR = 0,
      emTYPE_UINT8,
      emTYPE_INT8,
      emTYPE_UINT16,
      emTYPE_INT16,
      emTYPE_UINT32,
      emTYPE_INT32,
      emTYPE_UINT64,
      emTYPE_INT64,
      emTYPE_BLOB,
    };

    MYSQLValue()
    {
        m_dataType = emTYPE_STR;
        m_strData = "";
    }
    virtual ~MYSQLValue()
    {
    }
    //设置数据
    int SetData(const void* pData, uint32_t iLen, uint8_t dataType = emTYPE_STR)
    {
        if (!pData) {
            return 0;
        }
        m_strData.resize(iLen+1);
        memcpy((char*) m_strData.data(), pData, iLen);
        ((char*) m_strData.data())[iLen] = '\0';
        m_dataType = dataType;
        return 0;
    }
    int Size()
    {
        return m_strData.size()-1;
    }
    const char* Data()
    {
        return m_strData.data();
    }
    uint8_t DataType()
    {
        return m_dataType;
    }

    template<typename T>
    T as()
    {
        T asVal;
        pri_as(asVal);

        return asVal;
    }
    template<typename T>
    operator T()
    {
        return as<T>();
    }
private:
    int pri_as(char*& val)
    {
        val = (char*) Data();
        return 0;
    }

    int pri_as(const char*& val)
    {
        val = Data();
        return 0;
    }
    int pri_as(char& val)
    {
        int32_t tmp_val;
        int ret = pri_as(tmp_val);
        val = tmp_val;
        return ret;
    }
    int pri_as(unsigned char& val)
    {
        uint32_t tmp_val;
        int ret = pri_as(tmp_val);
        val = tmp_val;
        return ret;
    }
    int pri_as(std::string& val)
    {
        string tmp(Data(),Size());
        val = tmp;
        return 0;
    }
    template<typename T>
    int pri_as(T& val)
    {
//Add-Begin by dantezhu in 2011-03-15 20:15:19
//要不然如果是空串的话，转成的int是随机值
        if (this->Size()<=0) {
            val = 0;
            return 0;
        }
//Add-End
        stringstream ss;
        ss << m_strData;
        ss >> val;
        return 0;
    }
protected:
    string m_strData;
    uint8_t m_dataType;//数据类型
};
//绑定参数
class CBindParam: public MYSQLValue {
public:
    CBindParam() { }
    virtual ~CBindParam() { }

    void Bind(std::string& val)
    {
        SetData(val.c_str(),val.length(),emTYPE_STR);
    }
    void BindBlob(const void* pData, int iLen)
    {
        SetData(pData,iLen,emTYPE_BLOB);
    }
    void Bind(int8_t val)
    {
        SetData(&val, sizeof(val),emTYPE_INT8);
    }
    void Bind(uint8_t val)
    {
        SetData(&val, sizeof(val),emTYPE_UINT8);
    }
    void Bind(int16_t val)
    {
        SetData(&val, sizeof(val),emTYPE_INT16);
    }
    void Bind(uint16_t val)
    {
        SetData(&val, sizeof(val),emTYPE_UINT16);
    }
    void Bind(int32_t val)
    {
        SetData(&val, sizeof(val),emTYPE_INT32);
    }
    void Bind(uint32_t val)
    {
        SetData(&val, sizeof(val),emTYPE_UINT32);
    }
    void Bind(int64_t val)
    {
        SetData(&val, sizeof(val),emTYPE_INT64);
    }
    void Bind(uint64_t val)
    {
        SetData(&val, sizeof(val),emTYPE_UINT64);
    }
};

//一行结果集
class CMysqlResultRow {
public:
    CMysqlResultRow()
    {
        m_rowData.clear();
    }
    virtual ~CMysqlResultRow()
    {
    }
    void Clear()
    {
        m_rowData.clear();
    }
    void SetData(string filedName, const char* pData, int iLen)
    {
        MYSQLValue& value = m_rowData[filedName];
        value.SetData(pData, iLen);
    }
    MYSQLValue& operator[](string filedName)
    {
        return m_rowData[filedName];
    }
protected:
    map<string, MYSQLValue> m_rowData;
};

#ifndef MAX_SQL_LEN
#define MAX_SQL_LEN 12*1024
#endif
/*************************************************************/
class CDBWrap {
protected:
    CDBMySql m_clDatabase;
    CMysqlResultRow m_tmpRowData;
    std::shared_ptr<CMySQLTransaction> m_pTransaction;

public:
    CDBWrap();
    virtual ~CDBWrap();

public:
    bool dbOpen(string host, string user, string passwd, string db, unsigned int port);
    bool dbSelect(string db);
    void dbClose();
    bool ping();
    /*--->[ 开始事务 ]*/
    bool begin();
    /*--->[ 提交事务 ]*/
    bool commit();
    /*--->[ 回滚事务 ]*/
    bool rollback();

    // 执行一条sql语句
    bool ExeSql(const string& strSql);

    // 获得sql语句的结果数量
    int GetResNumExeSql(const string& strSql);

    // 获得影响行数
    int GetAffectedNumExeSql(const string& strSql);

    // 返回列名对应的值
    int Query(const string& strSql, vector<CMysqlResultRow>& vecData, int64_t& affectRow);
    bool ExeBindSql(const string& strSql, vector<CBindParam>& bindParams);

    // 插入数据
    int Insert(string tblName, SQLJoin& data);

    // 更新数据
    int Update(string tblName, SQLJoin& data, SQLJoin& where);

    // 插入或者更新
    int UpdateOrInsert(string tblName, SQLJoin& data);

    // 删除数据
    int Delete(string tblName, SQLJoin& where);

    // 插入数据
    static string GetInsertSql(string tblName, SQLJoin& data);

    // 更新数据
    static string GetUpdateSql(string tblName, SQLJoin& data, SQLJoin& where);

    // 插入或者更新
    static string GetUpdateOrInsertSql(string tblName, SQLJoin& data);

    // 删除数据
    static string GetDeleteSql(string tblName, SQLJoin& where);

    // select
    static string GetSelectSql(string tblName, SQLJoin& fileds, SQLJoin& where);

};


