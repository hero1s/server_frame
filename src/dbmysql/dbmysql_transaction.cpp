/*----------------- dbMySqlTransaction.cpp
*
* Copyright (C) 2013 MokyLin RXTL
* Author: Mokylin
* Version: 1.0
* Date: 2011/8/16 23:13:23
*--------------------------------------------------------------
*MySQL事务
*------------------------------------------------------------*/
#include <my_global.h>
#include "dbmysql/dbmysql.h"
/*************************************************************/
/*
**
*/
CMySQLTransaction::CMySQLTransaction(CDBMySql* pMySQL)
{
    if (!pMySQL)
        pMySQL = NULL;
    else
        m_pMySQL = pMySQL->m_pMySQL;

    m_bEndTrans = true;
}

/*
**
*/
CMySQLTransaction::CMySQLTransaction(MYSQL* mysql)
{
    m_pMySQL    = mysql;
    m_bEndTrans = true;
}

/*
**
*/
CMySQLTransaction::~CMySQLTransaction()
{
    rollback();
    if (m_pMySQL)
        mysql_autocommit(m_pMySQL, true);
}

/*
**开始事务
*/
bool CMySQLTransaction::begin()
{
    if (!m_pMySQL || !m_bEndTrans)
        return false;

    if (mysql_autocommit(m_pMySQL, false)==0)
    {
        m_bEndTrans = false;
        return true;
    }

#ifdef _MySQL_throw
    throw CMySQLException(m_pMySQL, 0, "mysql_autocommit()");
#endif
    return false;
}

/*
**提交事务
*/
bool CMySQLTransaction::commit()
{
    if (!m_pMySQL || m_bEndTrans)
        return false;

    if (mysql_commit(m_pMySQL)==0)
    {
        m_bEndTrans = true;
        return true;
    }

#ifdef _MySQL_throw
    throw CMySQLException(m_pMySQL, 0, "mysql_commit()");
#endif
    return false;
}

/*
**回滚事务
*/
bool CMySQLTransaction::rollback()
{
    if (!m_pMySQL || m_bEndTrans)
        return false;

    if (mysql_rollback(m_pMySQL)==0)
    {
        m_bEndTrans = true;
        return true;
    }

    return false;
}

