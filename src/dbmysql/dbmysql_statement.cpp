/*----------------- dbMySqlStatement.cpp
*
* Copyright (C) 2013 MokyLin RXTL
* Author: Mokylin
* Version: 1.0
* Date: 2011/8/16 23:12:51
*--------------------------------------------------------------
** 语句预处理
*执行步骤
*1.用msyql_stmt_init()创建预处理语句句柄。要想在服务器上准备预处理语句，可调用mysql_stmt_prepare()，并为其传递包含SQL语句的字符串。
*2.如果语句生成了结果集，调用mysql_stmt_result_metadata()以获得结果集元数据。虽然与包含查询返回列的结果集不同，该元数据本身也采用了结果集的形式。元数据结果集指明了结果中包含多少列，并包含每一列的信息。
*3.使用mysql_stmt_bind_param()设置任何参数的值。必须设置所有参数。否则，语句执行将返回错误，或生成无法预料的结果。
*4.调用mysql_stmt_execute()执行语句。
*5.如果语句生成了结果集，捆绑数据缓冲，通过调用mysql_stmt_bind_result()，检索行值。
*6.通过重复调用mysql_stmt_fetch()，按行将数据提取到缓冲区，直至未发现更多行为止。
*7.通过更改参数值并再次执行语句，重复步骤3到步骤6。
*------------------------------------------------------------*/
#include <my_global.h>
#include "dbmysql/dbmysql.h"
/*************************************************************/
/*
**
*/
CMySQLStatement::CMySQLStatement(MYSQL_STMT* pStatement)
{
    m_pStatement = pStatement;

    m_pParamLength = NULL;
    m_pParamBind   = NULL;
    m_ulParamCount = 0;

    m_ulRowCount    = 0;
    m_pResultBind   = NULL;
    m_ulResultCount = 0;
}

/*
**
*/
CMySQLStatement::~CMySQLStatement()
{
    close();
}

/*
**
*/
CMySQLStatement& CMySQLStatement::operator=(CMySQLStatement& clMySQLStatement)
{
    close();
    m_pStatement   = clMySQLStatement.m_pStatement;
    m_pParamLength = clMySQLStatement.m_pParamLength;
    m_pParamBind   = clMySQLStatement.m_pParamBind;
    m_ulParamCount = clMySQLStatement.m_ulParamCount;

    m_ulRowCount    = clMySQLStatement.m_ulRowCount;
    m_pResultBind   = clMySQLStatement.m_pResultBind;
    m_ulResultCount = clMySQLStatement.m_ulResultCount;

    /*因为需要晰构关闭必须置空*/
    clMySQLStatement.m_pParamLength = NULL;
    clMySQLStatement.m_pStatement   = NULL;
    clMySQLStatement.m_pParamBind   = NULL;
    clMySQLStatement.m_ulParamCount = 0;

    clMySQLStatement.m_ulRowCount    = 0;
    clMySQLStatement.m_pResultBind   = NULL;
    clMySQLStatement.m_ulResultCount = 0;

    return *this;
}

/*
**释放绑定缓冲
*/
void CMySQLStatement::freeBind()
{
    if (m_pParamLength)
        delete[] m_pParamLength;

    if (m_pParamBind)
        delete[] m_pParamBind;
    m_pParamBind   = NULL;
    m_ulParamCount = 0;

    if (m_pResultBind)
        delete[] m_pResultBind;
    m_pResultBind   = NULL;
    m_ulResultCount = 0;

    m_ulRowCount = 0;
}

/*
**关闭预处理语句
*/
bool CMySQLStatement::close()
{
    freeBind();
    freeResult();

    if (!m_pStatement)
        return false;

    if (mysql_stmt_close(m_pStatement)!=0)
        return false;

    m_pStatement = NULL;
    return true;
}

/*
**释放与执行预处理语句生成的结果集有关的内存
*/
bool CMySQLStatement::freeResult()
{
    if (!m_pStatement)
        return false;

    if (mysql_stmt_free_result(m_pStatement)==0)
        return true;

    return false;
}

/*
**格式化数据库指令并准备
*/
bool CMySQLStatement::cmd(const char* pCmd, ...)
{
    freeBind();
    if (!pCmd)
        return false;

    unsigned long ulCmdLength = 0;
    char          szCommand[16*1024];
    memset(szCommand, 0, sizeof(szCommand));

    va_list argptr;
    va_start(argptr, pCmd);
    ulCmdLength = dVsprintf(szCommand, sizeof(szCommand), pCmd, argptr);
    va_end(argptr);

    if (!m_pStatement)
        return false;

    /*为执行操作准备SQL字符串*/
    if (mysql_stmt_prepare(m_pStatement, szCommand, ulCmdLength)!=0)
    {
#ifdef _MySQL_throw
        throw CMySQLException(m_pStatement, 0, "mysql_stmt_prepare()");
#endif
        return false;
    }

    /*参数*/
    /*获得参数数量*/
    m_ulParamCount = mysql_stmt_param_count(m_pStatement);
    /*分配缓冲区*/
    if (m_ulParamCount)
    {
        m_pParamBind   = new MYSQL_BIND[m_ulParamCount];
        m_pParamLength = new unsigned long[m_ulParamCount];
        if (!m_pParamBind)
        {
#ifdef _MySQL_throw
            throw CMySQLException(m_pStatement, 0, "mysql_stmt_param_count()分配参数缓冲区失败!");
#endif
            return false;
        }
        memset(m_pParamBind, 0, sizeof(MYSQL_BIND)*m_ulParamCount);
        memset(m_pParamLength, 0, sizeof(unsigned long)*m_ulParamCount);
    }

    /*结果*/
    if (1)
    {
        /*返回关于最近语句的行数*/
        m_ulResultCount = mysql_stmt_field_count(m_pStatement);

    }
    else
    {
        /*取出结果集中信息*/
        MYSQL_RES* pResultMetadata = mysql_stmt_result_metadata(m_pStatement);
        if (pResultMetadata)
        {
            /*获得查询结果列数量*/
            m_ulResultCount = mysql_num_fields(pResultMetadata);
            mysql_free_result(pResultMetadata);
        }
    }

    /*分配缓冲区*/
    if (m_ulResultCount)
    {
        m_pResultBind = new MYSQL_BIND[m_ulResultCount];
        if (!m_pResultBind)
        {
#ifdef _MySQL_throw
            throw CMySQLException(m_pStatement, 0, "分配结果缓冲区失败!");
#endif
            return false;
        }
        memset(m_pResultBind, 0, sizeof(MYSQL_BIND)*m_ulResultCount);
    }

    return true;
}

/*
**执行(execute)
*/
bool CMySQLStatement::execute()
{
    if (!m_pStatement)
        return false;

    if (mysql_stmt_execute(m_pStatement))
    {
#ifdef _MySQL_throw
        throw CMySQLException(m_pStatement, 0, "mysql_stmt_execute()");
#endif
        return false;
    }

    /*设置返回结果更新字段最大长度值*/
    my_bool bUpdateMaxLength = 1;
    if (mysql_stmt_attr_get(m_pStatement, STMT_ATTR_UPDATE_MAX_LENGTH, &bUpdateMaxLength)==0)
    {
        if (bUpdateMaxLength!=1)
        {
            bUpdateMaxLength = 1;
            /*如果设为1：更新mysql_stmt_store_result()中的元数据MYSQL_FIELD->max_length*/
            if (mysql_stmt_attr_set(m_pStatement, STMT_ATTR_UPDATE_MAX_LENGTH, &bUpdateMaxLength))
            {
                /*如果选项未知，返回非0值*/
            }
        }
    }

    /*获得结果缓冲到客户端*/
    if (mysql_stmt_store_result(m_pStatement))
    {
#ifdef _MySQL_throw
        throw CMySQLException(m_pStatement, 0, "mysql_stmt_store_result()");
#endif
        return false;
    }
    /*获得结果行数*/
    m_ulRowCount = mysql_stmt_num_rows(m_pStatement);

    return true;
}

/*
**预处理语句复位为完成准备后的状态
*/
bool CMySQLStatement::reset()
{
    if (!m_pStatement)
        return false;

    if (mysql_stmt_reset(m_pStatement)==0)
        return true;

#ifdef _MySQL_throw
    throw CMySQLException(m_pStatement, 0, "mysql_stmt_reset()");
#endif
    return false;
}

/*
**获得影响/改变行数
*/
my_ulonglong CMySQLStatement::getRowAffected()
{
    if (!m_pStatement)
        return 0;

    /*返回由预处理语句UPDATE、DELETE或INSERT变更、删除或插入的行数目。*/
    return mysql_stmt_affected_rows(m_pStatement);
}

/*
**获得AUTO_INCREMENT列生成的ID
*/
my_ulonglong CMySQLStatement::getInsertIncrement()
{
    if (!m_pStatement)
        return 0;

    /*对于预处理语句的AUTO_INCREMENT列，返回生成的ID*/
    return mysql_stmt_insert_id(m_pStatement);
}

/*
**绑定参数
*/
bool CMySQLStatement::bindParams()
{
    if (!m_pStatement)
        return false;

    /*绑定缓冲*/
    if (mysql_stmt_bind_param(m_pStatement, m_pParamBind)==0)
        return true;

#ifdef _MySQL_throw
    throw CMySQLException(m_pStatement, 0, "mysql_stmt_bind_param()");
#endif
    return false;
}

/*
**发送长数据到服务器
*/
bool CMySQLStatement::sendLongData(unsigned int parameter_number, const char* data, unsigned long length)
{
    if (!m_pStatement)
        return false;

    if (parameter_number>=m_ulParamCount)
        return false;

    if (mysql_stmt_send_long_data(m_pStatement, parameter_number, data, length)==0)
        return true;

#ifdef _MySQL_throw
    throw CMySQLException(m_pStatement, 0, "mysql_stmt_send_long_data()");
#endif
    return false;
}

/*
**绑定结果集
*/
bool CMySQLStatement::bindResults()
{
    if (!m_pStatement || !m_pResultBind)
        return false;

    return (mysql_stmt_bind_result(m_pStatement, m_pResultBind)==0);
}

/*
**存在行数据
*/
bool CMySQLStatement::rowMore()
{
    if (!m_pStatement)
        return false;

    int nRes = mysql_stmt_fetch(m_pStatement);
    if (nRes==MYSQL_NO_DATA)
        return false;

    if (nRes!=0 && mysql_stmt_errno(m_pStatement)!=0)
    {
#ifdef _MySQL_throw
        throw CMySQLException(m_pStatement, 0, "mysql_stmt_fetch()");
#endif
        return false;
    }

    return true;
}

/*
**取出第几行数据
*/
bool CMySQLStatement::dataSeek(my_ulonglong offset)
{
    if (!m_pStatement)
        return false;

    mysql_stmt_data_seek(m_pStatement, offset);

    return rowMore();
}

/*
**从当前结果集行获取1列
*/
bool CMySQLStatement::getColume(MYSQL_BIND* bind, unsigned int column, unsigned long offset)
{
    if (!m_pStatement || !bind)
        return false;

    int nRes = mysql_stmt_fetch_column(m_pStatement, bind, column, offset);
    if (nRes==0)
        return true;

    if (nRes==CR_NO_DATA)
        return false;

#ifdef _MySQL_throw
    throw CMySQLException(m_pStatement, 0, "mysql_stmt_fetch_column()错误的列序号!");/*CR_INVALID_PARAMETER_NO*/
#endif

    return false;
}

/*
**绑定参数
*/
bool CMySQLStatement::bindParam(unsigned int uField, enum_field_types buffer_type, void* buffer, unsigned long length,
        my_bool* is_null)
{
    if (uField>=m_ulParamCount || !m_pStatement)
    {
#ifdef _MySQL_throw
        throw CMySQLException(m_pStatement, 0, "CMySQLStatement::bindParam()");
#endif
        return false;
    }

    m_pParamLength[uField] = length;

    m_pParamBind[uField].buffer_type = buffer_type;
    m_pParamBind[uField].buffer      = buffer;
    m_pParamBind[uField].length      = &m_pParamLength[uField];
    m_pParamBind[uField].is_null     = is_null;        /*如果值为NULL，该变量为“真”*/

    return true;
}

/*
**
*/
bool
CMySQLStatement::bindParam_Int(unsigned int uField, enum_field_types buffer_type, my_bool is_unsigned, void* buffer,
        my_bool* is_null)
{
    if (uField>=m_ulParamCount || !m_pStatement)
    {
#ifdef _MySQL_throw
        throw CMySQLException(m_pStatement, 0, "绑定参数溢出!");
#endif
        return false;
    }

    m_pParamBind[uField].buffer_type = buffer_type;
    m_pParamBind[uField].is_unsigned = is_unsigned;    /*是否为无符号*/
    m_pParamBind[uField].buffer      = buffer;
    m_pParamBind[uField].is_null     = is_null;        /*如果值为NULL，该变量为“真”*/

    return true;
}

/*
**绑定结果
*/
bool CMySQLStatement::bindResult(unsigned int uField, enum_field_types buffer_type, void* buffer,
        unsigned long buffer_length, my_bool* is_null)
{
    if (uField>=m_ulResultCount || !m_pStatement)
    {
#ifdef _MySQL_throw
        throw CMySQLException(m_pStatement, 0, "绑定结果溢出!");
#endif
        return false;
    }

    m_pResultBind[uField].buffer_type   = buffer_type;
    m_pResultBind[uField].buffer_length = buffer_length;
    m_pResultBind[uField].buffer        = buffer;
    m_pResultBind[uField].is_null       = is_null;        /*如果值为NULL，该变量为“真”*/

    return true;
}

/*
**绑定结果整型
*/
bool
CMySQLStatement::bindResult_Int(unsigned int uField, enum_field_types buffer_type, my_bool is_unsigned, void* buffer,
        my_bool* is_null)
{
    if (uField>=m_ulResultCount || !m_pStatement)
    {
#ifdef _MySQL_throw
        throw CMySQLException(m_pStatement, 0, "CMySQLStatement::bindResult_Int()");
#endif
        return false;
    }

    m_pResultBind[uField].buffer_type = buffer_type;
    m_pResultBind[uField].is_unsigned = is_unsigned;    /*是否为无符号*/
    m_pResultBind[uField].buffer      = buffer;
    m_pResultBind[uField].is_null     = is_null;        /*如果值为NULL，该变量为“真”*/

    return true;
}



