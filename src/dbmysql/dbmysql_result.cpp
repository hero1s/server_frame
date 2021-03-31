/*----------------- dbMySqlResult.cpp
*
* Copyright (C) 2013 MokyLin RXTL
* Author: Mokylin
* Version: 1.0
* Date: 2011/8/16 23:12:35
*--------------------------------------------------------------
*数据库查询结果集类实现
*------------------------------------------------------------*/
#include "dbmysql/dbmysql.h"
#include <my_global.h>
/*************************************************************/
/*
**
*/
CMySQLResult::CMySQLResult(MYSQL* mysql, MYSQL_RES* pResult)
{
    m_pMySQL = mysql;
    m_pResult = NULL;

    initialize(pResult);
}

/*
**
*/
CMySQLResult::~CMySQLResult()
{
    freeResult();
}

/*
**
*/
CMySQLResult& CMySQLResult::operator=(CMySQLResult& clMySQLResult)
{
    freeResult();

    m_pMySQL = clMySQLResult.m_pMySQL;
    m_pResult = clMySQLResult.m_pResult;
    m_pRow = clMySQLResult.m_pRow;
    m_ulRowCount = clMySQLResult.m_ulRowCount;
    m_uFiledCount = clMySQLResult.m_uFiledCount;
    m_pulFiledsLength = clMySQLResult.m_pulFiledsLength;

    /*因为需要晰构释放必须置空*/
    clMySQLResult.m_pMySQL = NULL;
    clMySQLResult.m_pResult = NULL;
    clMySQLResult.m_pRow = NULL;
    clMySQLResult.m_ulRowCount = 0;
    clMySQLResult.m_uFiledCount = 0;
    clMySQLResult.m_pulFiledsLength = NULL;

    return *this;
}

/*
**初始化结果集
*/
void CMySQLResult::initialize(MYSQL_RES* pResult)
{
    freeResult();
    m_pResult = pResult;

    if (m_pResult) {
        /*结果集行数*/
        m_ulRowCount = mysql_num_rows(m_pResult);
        /*结果集列数*/
        m_uFiledCount = mysql_num_fields(m_pResult);
    }
}

/*
**释放结果集使用的内存
*/
void CMySQLResult::freeResult()
{
    if (m_pResult)
        mysql_free_result(m_pResult);

    m_pResult = NULL;

    m_pRow = NULL;
    m_ulRowCount = 0;
    m_uFiledCount = 0;
    m_pulFiledsLength = NULL;
}

/*
**返回上次字段光标的位置
*/
MYSQL_FIELD_OFFSET CMySQLResult::fieldTell()
{
    if (!m_pResult)
        return 0;

    return mysql_field_tell(m_pResult);
}

/*
**将列光标置于指定的列(开始为0到FieldCount)
*/
MYSQL_FIELD_OFFSET CMySQLResult::fieldSeek(MYSQL_FIELD_OFFSET offset)
{
    if (!m_pResult)
        return false;

    if (offset > m_uFiledCount - 1)
        return false;

    return mysql_field_seek(m_pResult, offset);
}

/*
**检索关于结果集中所有列的信息(下一个字段)
*/
MYSQL_FIELD* CMySQLResult::fieldFetch()
{
    if (!m_pResult)
        return NULL;

    //pField->db;			/*字段源自的数据*/
    //pField->table;		/*表的名称*/
    //pField->name;			/*字段名称*/
    //pField->length;		/*字段的宽度*/
    //pField->max_length;	/*字段最大宽度*/
    //pField->flags;		/*字段标志(IS_NOT_NULL(pField->flags)),IS_BLOB(pField->flags)*/
    //pField->type;			/*字段类型*/

    return mysql_fetch_field(m_pResult);
}

/*
**结果集内某列的信息(fieldnr = 0到FieldCount-1)
*/
MYSQL_FIELD* CMySQLResult::fieldFetchDirect(unsigned int fieldnr)
{
    if (!m_pResult)
        return NULL;

    if (fieldnr > m_uFiledCount - 1)
        return NULL;

    return mysql_fetch_field_direct(m_pResult, fieldnr);
}

/*
**返回所有字段结构的数组
*/
MYSQL_FIELD* CMySQLResult::fieldFetchs()
{
    if (!m_pResult)
        return NULL;

    return mysql_fetch_fields(m_pResult);
}

/*
**获得列数据长度
*/
unsigned long CMySQLResult::getFiledDataLength(unsigned int uField)
{
    if (!m_pResult)
        return 0;

    if (uField > m_uFiledCount - 1)
        return 0;

    if (!m_pulFiledsLength || !m_pulFiledsLength[uField])
        return 0;

    return m_pulFiledsLength[uField];
}

/*
**下移一行数据
*/
bool CMySQLResult::rowMore()
{ /*从结果集中获取下一行(row[i])*/
    if (!m_pResult)
        return false;

    m_pulFiledsLength = NULL;
    m_pRow = mysql_fetch_row(m_pResult);
    if (!m_pRow)
        return false;

    m_pulFiledsLength = mysql_fetch_lengths(m_pResult);

    return true;
}

/*
**是否已到结果行尾
*/
bool CMySQLResult::rowEof()
{
    if (!m_pResult)
        return true;

    /*该函数已不再被重视，可以使用mysql_errno()或mysql_error()取而代之。*/
    return (mysql_eof(m_pResult) == 0);
}

/*
**取出第几行数据
*/
bool CMySQLResult::dataSeek(my_ulonglong offset)
{
    if (!m_pResult)
        return false;

    if (offset > m_ulRowCount - 1)
        return false;

    /*在查询结果集中查找属性行编号。
    仅应与mysql_store_result()联合使用,范围从0到mysql_num_rows(result)-1*/
    mysql_data_seek(m_pResult, offset);

    return rowMore();
}

/*
**当前行光标
*/
MYSQL_ROWS* CMySQLResult::rowTell()
{ /*返回行光标位置*/
    if (!m_pResult)
        return NULL;
    ;

    return mysql_row_tell(m_pResult);
}

/*
**移动行光标
*/
MYSQL_ROWS* CMySQLResult::rowSeek(MYSQL_ROWS* offset)
{
    if (!m_pResult || !offset)
        return NULL;

    /*使用从mysql_row_tell()返回的值，查找结果集中的行偏移*/
    return mysql_row_seek(m_pResult, offset);
}

/*
**获取当前行字段数据/字符串数据
*/
const char* CMySQLResult::getData(unsigned int uField, const char* pDefault)
{
    if (!m_pResult)
        return NULL;

    if (uField > m_uFiledCount - 1)
        return NULL;

    if (!m_pRow && !rowMore())
        return NULL;

    if (!m_pRow[uField])
        return pDefault;

    return m_pRow[uField];
}
