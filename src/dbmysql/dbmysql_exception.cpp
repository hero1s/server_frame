/*----------------- dbMySqlException.cpp
*
* Copyright (C) 2013 MokyLin RXTL
* Author: Mokylin
* Version: 1.0
* Date: 2011/8/16 23:12:13
*--------------------------------------------------------------
*MySQL异常处理
*------------------------------------------------------------*/

#include "dbmysql/dbmysql.h"
#include "spdlog/spdlog.h"
#include "utility/comm_macro.h"
#include <my_global.h>

/*************************************************************/

//-------------------------------------------------------------
//------------------------------
CMySQLException::CMySQLException(MYSQL* mysql, unsigned int code, const char* pMsg)
{
    this->uCode = code;
    memset(this->szMsg, 0, sizeof(this->szMsg));
    memset(this->szError, 0, sizeof(this->szError));
    memset(this->szSqlState, 0, sizeof(this->szSqlState));

    if (pMsg)
        dStrcpy(this->szMsg, sizeof(this->szMsg), pMsg);

    if (mysql) {
        /*返回上次调用的MySQL函数的错误编号*/
        if (!code)
            this->uCode = mysql_errno(mysql);

        /*返回上次调用的MySQL函数的错误消息*/
        dStrcpy(this->szError, sizeof(this->szError), mysql_error(mysql));
        /*返回关于上一个错误的SQLSTATE错误代码*/
        dStrcpy(this->szSqlState, sizeof(this->szSqlState), mysql_sqlstate(mysql));
    }
    LOG_CRITIC("MYSQL Exception:{} {} ", code, pMsg);
}

//-------------------------------------------------------------
//------------------------------
CMySQLException::CMySQLException(MYSQL_STMT* stmt, unsigned int code, const char* pMsg)
{
    this->uCode = code;
    memset(this->szMsg, 0, sizeof(this->szMsg));
    memset(this->szError, 0, sizeof(this->szError));
    memset(this->szSqlState, 0, sizeof(this->szSqlState));

    if (pMsg)
        dStrcpy(this->szMsg, sizeof(this->szMsg), pMsg);

    if (stmt) {
        if (!code)
            this->uCode = mysql_stmt_errno(stmt); /*返回上次语句执行的错误编号*/

        /*返回上次语句执行的错误消息*/
        dStrcpy(this->szError, sizeof(this->szError), mysql_stmt_error(stmt));
        /*返回关于上次语句执行的SQLSTATE错误代码*/
        dStrcpy(this->szSqlState, sizeof(this->szSqlState), mysql_stmt_sqlstate(stmt));
    }
    LOG_CRITIC("MYSQL Exception:{} {} ", code, pMsg);
}
