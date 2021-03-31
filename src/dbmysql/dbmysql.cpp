/*----------------- dbMySql.cpp
*
* Copyright (C) 2013 MokyLin RXTL
* Author: Mokylin
* Version: 1.0
* Date: 2011/8/16 23:09:37
*--------------------------------------------------------------
*CDBMySql类实现
*------------------------------------------------------------*/
#include "dbmysql/dbmysql.h"
#include <my_global.h>
/*#pragma comment(lib, "libmysql.lib")*/
/*************************************************************/
/*
**
*/
CDBMySql::CDBMySql()
{
    m_pMySQL = NULL;
    m_bConnected = false;

    mysql_server_init(0, NULL, NULL); /*可交由mysql_init()会自动调用*/
    initialize(true, 10);
}

/*
**
*/
CDBMySql::~CDBMySql()
{
    close();
    m_pMySQL = NULL;
    mysql_server_end();
}

/*
**初始化数据库链接
*/
void CDBMySql::initialize(my_bool bReConnect, unsigned int uConnectTimeout, const char* pCharsetName)
{
    m_bReConnect = bReConnect;
    m_uConnectTimeout = uConnectTimeout;

    memset(m_szCharsetName, 0, sizeof(m_szCharsetName));

#ifdef WIN32
    dSprintf(m_szCharsetName, sizeof(m_szCharsetName), "%s", pCharsetName ? pCharsetName : "gbk");
#else //WIN32
    dSprintf(m_szCharsetName, sizeof(m_szCharsetName), "%s", pCharsetName ? pCharsetName : "utf8");
#endif //WIN32

    m_ulCmdLength = 0;
    memset(m_szCommand, 0, sizeof(m_szCommand));
}

/*
**打开链接数据库
*/
bool CDBMySql::open(const char* host, const char* user, const char* passwd, const char* db, unsigned int port,
    const char* unixSocket, unsigned long clientFlag)
{
    m_bConnected = false;
    /*分配或初始化(MYSQL)对像*/
    m_pMySQL = mysql_init(m_pMySQL);
    if (!m_pMySQL) {
#ifdef _MySQL_throw
        throw CMySQLException(m_pMySQL, 0, "mysql_init()");
#endif
        return false;
    }
    /*mysql_options:参数enum mysql_option
    |选项								|参量类型			|功能																							|
    |-----------------------------------|-------------------|-----------------------------------------------------------------------------------------------|
    |MYSQL_INIT_COMMAND					|char*				|连接到MySQL服务器时将执行的命令。再次连接时将自动地再次执行。									|
    |-----------------------------------|-------------------|-----------------------------------------------------------------------------------------------|
    |MYSQL_OPT_COMPRESS					|未使用				|使用压缩客户端／服务器协议																		|
    |-----------------------------------|-------------------|-----------------------------------------------------------------------------------------------|
    |MYSQL_OPT_CONNECT_TIMEOUT			|unsigned int*		|以秒为单位的连接超时。																			|
    |-----------------------------------|-------------------|-----------------------------------------------------------------------------------------------|
    |MYSQL_OPT_GUESS_CONNECTION			|未使用				|对于与libmysqld链接的应用程序，允许库“猜测”是否使用嵌入式服务器或远程服务器。				|
    |									|					|“猜测”表示，如果设置了主机名但不是本地主机，将使用远程服务器。该行为是默认行为。				|
    |									|					|可使用MYSQL_OPT_USE_EMBEDDED_CONNECTION 和MYSQL_OPT_USE_REMOTE_CONNECTION覆盖它。				|
    |									|					|对于与libmysqlclient链接的应用程序，该选项将被忽略。											|
    |-----------------------------------|-------------------|-----------------------------------------------------------------------------------------------|
    |MYSQL_OPT_LOCAL_INFILE				|指向单元的可选指针	|如果未给定指针，或指针指向“unsigned int != 0”，将允许命令LOAD LOCAL INFILE。					|
    |-----------------------------------|-------------------|-----------------------------------------------------------------------------------------------|
    |MYSQL_OPT_NAMED_PIPE				|未使用				|使用命名管道连接到NT平台上的MySQL服务器。														|
    |-----------------------------------|-------------------|-----------------------------------------------------------------------------------------------|
    |MYSQL_OPT_PROTOCOL					|unsigned int*		|要使用的协议类型。应是mysql.h中定义的mysql_protocol_type的枚举值之一。							|
    |-----------------------------------|-------------------|-----------------------------------------------------------------------------------------------|
    |MYSQL_OPT_READ_TIMEOUT				|unsigned int*		|从服务器读取信息的超时（目前仅在Windows平台的TCP/IP连接上有效）。								|
    |-----------------------------------|-------------------|-----------------------------------------------------------------------------------------------|
    |MYSQL_OPT_RECONNECT				|my_bool*			|如果发现连接丢失，启动或禁止与服务器的自动再连接。从MySQL 5.0.3开始，							|
    |									|					|默认情况下禁止再连接，这是5.0.13中的新选项，提供了一种以显式方式设置再连接行为的方法。			|
    |-----------------------------------|-------------------|-----------------------------------------------------------------------------------------------|
    |MYSQL_OPT_SET_CLIENT_IP			|char*				|对于与libmysqld链接的应用程序（具备鉴定支持特性的已编译libmysqld），它意味着，出于鉴定目的，	|
    |									|					|用户将被视为从指定的IP地址（指定为字符串）进行连接。对于与libmysqlclient链接的应用程序，		|
    |									|					|该选项将被忽略。																				|
    |-----------------------------------|-------------------|-----------------------------------------------------------------------------------------------|
    |MYSQL_OPT_USE_EMBEDDED_CONNECTION	|未使用				|对于与libmysqld链接的应用程序，对于连接来说，它将强制使用嵌入式服务器。						|
    |									|					|对于与libmysqlclient链接的应用程序，该选项将被忽略。											|
    |-----------------------------------|-------------------|-----------------------------------------------------------------------------------------------|
    |MYSQL_OPT_USE_REMOTE_CONNECTION	|未使用				|对于与libmysqld链接的应用程序，对于连接来说，它将强制使用远程服务器。							|
    |									|					|对于与libmysqlclient链接的应用程序，该选项将被忽略。											|
    |-----------------------------------|-------------------|-----------------------------------------------------------------------------------------------|
    |MYSQL_OPT_USE_RESULT				|未使用				|不使用该选项。																					|
    |-----------------------------------|-------------------|-----------------------------------------------------------------------------------------------|
    |MYSQL_OPT_WRITE_TIMEOUT			|unsigned int*		|写入服务器的超时（目前仅在Windows平台的TCP/IP连接上有效）。									|
    |-----------------------------------|-------------------|-----------------------------------------------------------------------------------------------|
    |MYSQL_READ_DEFAULT_FILE			|char*				|从命名选项文件而不是从my.cnf读取选项。															|
    |-----------------------------------|-------------------|-----------------------------------------------------------------------------------------------|
    |MYSQL_READ_DEFAULT_GROUP			|char*				|从my.cnf或用MYSQL_READ_DEFAULT_FILE指定的文件中的命名组读取选项。								|
    |-----------------------------------|-------------------|-----------------------------------------------------------------------------------------------|
    |MYSQL_REPORT_DATA_TRUNCATION		|my_bool*			|通过MYSQL_BIND.error，对于预处理语句，允许或禁止通报数据截断错误（默认为禁止）。				|
    |-----------------------------------|-------------------|-----------------------------------------------------------------------------------------------|
    |MYSQL_SECURE_AUTH					|my_bool*			|是否连接到不支持密码混编功能的服务器，在MySQL 4.1.1和更高版本中，使用了密码混编功能。			|
    |-----------------------------------|-------------------|-----------------------------------------------------------------------------------------------|
    |MYSQL_SET_CHARSET_DIR				|char*				|指向包含字符集定义文件的目录的路径名。															|
    |-----------------------------------|-------------------|-----------------------------------------------------------------------------------------------|
    |MYSQL_SET_CHARSET_NAME				|char*				|用作默认字符集的字符集的名称。																	|
    |-----------------------------------|-------------------|-----------------------------------------------------------------------------------------------|
    |MYSQL_SHARED_MEMORY_BASE_NAME		|char*				|命名为与服务器进行通信的共享内存对象。应与你打算连接的mysqld服务器使用的选项					|
    |									|					|“-shared-memory-base-name”相同。																|
    -------------------------------------------------------------------------------------------------------------------------------------------------------*/
    /*设置链接超时为3秒*/
    if (mysql_options(m_pMySQL, MYSQL_OPT_CONNECT_TIMEOUT, (char*)&m_uConnectTimeout)) {
#ifdef _MySQL_throw
        throw CMySQLException(m_pMySQL, 0, "mysql_options(MYSQL_OPT_CONNECT_TIMEOUT)");
#endif
    }

    unsigned int uTimeout = 10;
    if (mysql_options(m_pMySQL, MYSQL_OPT_READ_TIMEOUT, (char*)&uTimeout)) {
#ifdef _MySQL_throw
        throw CMySQLException(m_pMySQL, 0, "mysql_options(MYSQL_OPT_CONNECT_TIMEOUT)");
#endif
    }
    if (mysql_options(m_pMySQL, MYSQL_OPT_WRITE_TIMEOUT, (char*)&uTimeout)) {
#ifdef _MySQL_throw
        throw CMySQLException(m_pMySQL, 0, "mysql_options(MYSQL_OPT_CONNECT_TIMEOUT)");
#endif
    }

    /*自动重链*/
    if (mysql_options(m_pMySQL, MYSQL_OPT_RECONNECT, &m_bReConnect)) {
#ifdef _MySQL_throw
        throw CMySQLException(m_pMySQL, 0, "mysql_options(MYSQL_OPT_RECONNECT)");
#endif
    }
    /*设置字符集*/
    if (!m_szCharsetName[0]) {
        dSprintf(m_szCharsetName, sizeof(m_szCharsetName), "%s", "gbk");
    }

    if (mysql_options(m_pMySQL, MYSQL_SET_CHARSET_NAME, m_szCharsetName)) {
#ifdef _MySQL_throw
        throw CMySQLException(m_pMySQL, 0, "mysql_options(MYSQL_SET_CHARSET_NAME)");
#endif
    }
    /*使用压缩协议*/
    if (mysql_options(m_pMySQL, MYSQL_OPT_COMPRESS, NULL)) {
#ifdef _MySQL_throw
        throw CMySQLException(m_pMySQL, 0, "mysql_options(MYSQL_OPT_COMPRESS)");
#endif
    }
    /*允许通报数据截断错误*/
    my_bool bReportTruncation = true;
    if (mysql_options(m_pMySQL, MYSQL_REPORT_DATA_TRUNCATION, &bReportTruncation)) {
#ifdef _MySQL_throw
        throw CMySQLException(m_pMySQL, 0, "mysql_options(MYSQL_REPORT_DATA_TRUNCATION)");
#endif
    }

    /*协议类型MYSQL_OPT_PROTOCOL(mysql_protocol_type)*/
    /*读取超时MYSQL_OPT_READ_TIMEOUT(unsigned int*)*/
    /*写入超时MYSQL_OPT_WRITE_TIMEOUT(unsigned int*)*/

    /*host == NULL | "localhost"[与本地主机连接]*/
    /*user == NULL | ""			[当前用户 UNIX是当前的登录名 Windows ODBC必须明确当前用户名]*/
    /*passwd == NULL			[按特定方式设置MySQL权限,用户是否有密码,用户将获得不同的权限]*/
    /*db == NULL				[连接会将默认的数据库设为该值]*/
    /*port != 0					其值将用作TCP/IP连接的端口号*/
    /*unixSocket != NULL		该字符串描述了应使用的套接字或命名管道*/
    /*clientFlag */
    clientFlag |= CLIENT_MULTI_STATEMENTS; /*允许单个字符串内发送多条语句(由';'隔开)*/
    clientFlag |= CLIENT_MULTI_RESULTS; /*允许多结果集*/
    clientFlag |= CLIENT_LOCAL_FILES; /*允许LOAD DATA LOCAL处理功能*/

    if (!mysql_real_connect(m_pMySQL, host, user, passwd, db, port, unixSocket, clientFlag)) {
#ifdef _MySQL_throw
        throw CMySQLException(m_pMySQL, 0, "mysql_real_connect");
#endif
        close();
        return false;
    }

    /*启动多语句处理*/
    if (mysql_set_server_option(m_pMySQL, MYSQL_OPTION_MULTI_STATEMENTS_ON)) {
#ifdef _MySQL_throw
        throw CMySQLException(m_pMySQL, 0, "mysql_options(MYSQL_OPTION_MULTI_STATEMENTS_ON)");
#endif
    }

    m_bConnected = true;
    return true;
}

/*
**关闭数据库链接
*/
void CDBMySql::close()
{
    if (!m_pMySQL)
        return;

    m_bConnected = false;
    mysql_close(m_pMySQL);
    m_pMySQL = NULL;
}

/*
**是否链接SQL(如断开会自动链接)
*/
bool CDBMySql::ping()
{
    if (!m_pMySQL)
        return false;

    int32_t nRes = mysql_ping(m_pMySQL);
    if (nRes == 0)
        return true;

#ifdef _MySQL_throw
    static char szMsg[128] = { 0 };
    dSprintf(szMsg, sizeof(szMsg), "mysql_ping(%d)", nRes);

    throw CMySQLException(m_pMySQL, 0, szMsg);
#endif
    return false;
}

/*
**是否启动多语句处理
*/
bool CDBMySql::multiSentence(bool bMore)
{
    if (!m_pMySQL)
        return false;

    if (!mysql_set_server_option(m_pMySQL,
            bMore ? MYSQL_OPTION_MULTI_STATEMENTS_ON : MYSQL_OPTION_MULTI_STATEMENTS_OFF))
        return true;

#ifdef _MySQL_throw
    throw CMySQLException(m_pMySQL, 0, "mysql_set_server_option()");
#endif
    return false;
}

/*
**设置当前链接字符集
*/
bool CDBMySql::setConnectCharacter(const char* charsetName)
{
    if (!charsetName || !*charsetName || !ping())
        return false;

    return (mysql_set_character_set(m_pMySQL, charsetName) == 0);
}

/*
**设置当前字符集
*/
bool CDBMySql::setCharacter(const char* charsetName)
{
    if (!charsetName)
        return false;

    try {
        CMySQLMultiFree clMultiFree(this);

        this->cmd("SET names '%s';SET CHARACTER_SET '%s'", charsetName, charsetName);
        if (this->execute())
            return true;
    } catch (CMySQLException& e) {
    }

    return false;
}

/*
**当前连接默认字符集
*/
const char* CDBMySql::getConnectCharacter()
{
    if (!ping())
        return "";

    return mysql_character_set_name(m_pMySQL);
}

bool CDBMySql::getServerInfo()
{
    if (!ping())
        return false;

    dPrintf("mysql_get_host_info(%s)\n", mysql_get_host_info(m_pMySQL));
    dPrintf("以整数形式返回服务器的版本号(%d)\n", mysql_get_server_version(m_pMySQL));
    dPrintf("返回服务器的版本号(%s)\n", mysql_get_server_info(m_pMySQL));
    dPrintf("以字符串形式返回服务器状态(%s)\n", mysql_stat(m_pMySQL));
    dPrintf("最后查询信息(%s)\n", mysql_info(m_pMySQL));

    return true;
}

bool CDBMySql::getClientInfo()
{
    if (m_pMySQL)
        dPrintf("返回连接所使用的协议版本。(%d)\n", mysql_get_proto_info(m_pMySQL));

    dPrintf("以字符串形式返回客户端版本信息。(%s)\n", mysql_get_client_info());
    dPrintf("以整数形式返回客户端版本信息(%d)\n", mysql_get_client_version());

    return true;
}

/*
**显示当前服务器线程的列表
*/
CMySQLResult CDBMySql::showProcessesList()
{
    if (!ping())
        return CMySQLResult();

    MYSQL_RES* pResult = mysql_list_processes(m_pMySQL);
    if (!pResult) {
        /*错误*/
        /*CR_COMMANDS_OUT_OF_SYNC	以不恰当的顺序执行了命令。
        CR_SERVER_GONE_ERROR		MySQL服务器不可用。
        CR_SERVER_LOST				在查询过程中，与服务器的连接丢失。
        CR_UNKNOWN_ERROR			出现未知错误。 */
    }

    return CMySQLResult(m_pMySQL, pResult);
}

/*
**获得当前连接的线程ID
*/
unsigned long CDBMySql::getConnectThreadID()
{
    if (!ping())
        return 0;

    return mysql_thread_id(m_pMySQL);
}

/*
**获得最后一次SQL语句生成的告警数
*/
unsigned int CDBMySql::getLastWarningCount()
{
    if (!m_pMySQL)
        return 0;

    return mysql_warning_count(m_pMySQL);
}

/*
**获得执行查询的字符串信息
*/
const char* CDBMySql::getExecuteInfo()
{
    if (!m_pMySQL)
        return NULL;

    return mysql_info(m_pMySQL);
}

/*
**当前连接默认字符集信息
*/
bool CDBMySql::getConnectCharacter(MY_CHARSET_INFO& sInfo)
{
    memset(&sInfo, 0, sizeof(sInfo));
    if (!m_pMySQL)
        return false;

    mysql_get_character_set_info(m_pMySQL, &sInfo);

    dPrintf("字符集信息:\n");
    dPrintf("字符集名: %s\n", sInfo.name);
    dPrintf("校对名: %s\n", sInfo.csname);
    dPrintf("注释: %s\n", sInfo.comment);
    dPrintf("目录: %s\n", sInfo.dir);
    dPrintf("多字节字符最小长度: %d\n", sInfo.mbminlen);
    dPrintf("多字节字符最大长度: %d\n", sInfo.mbmaxlen);

    return true;
}

/*
**更改连接上的用户和数据库
*/
bool CDBMySql::changeConnectUser(const char* user, const char* passwd, const char* db)
{
    my_bool bRes = mysql_change_user(m_pMySQL, user, passwd, db);
    if (bRes == 0)
        return true;

    return false;
}

/*
**数据库选择
*/
bool CDBMySql::dbSelect(const char* db)
{
    if (!db || !*db || !ping())
        return false;

    return (mysql_select_db(m_pMySQL, db) == 0);
}

/*
**数据库创建
*/
bool CDBMySql::dbCreate(const char* db, const char* charsetName, const char* collationName, bool exist)
{
    if (!db || !*db || !ping())
        return false;

    /*该函数已不再被重视，使用SQL语句CREATE DATABASE取而代之。*/
    /*return (mysql_create_db(m_pMySQL,db) == 0);*/

    dSprintf(m_szCommand, sizeof(m_szCommand), "CREATE DATABASE");

    if (exist)
        dSprintf(m_szCommand, sizeof(m_szCommand), "%s IF NOT EXISTS", m_szCommand);

    dSprintf(m_szCommand, sizeof(m_szCommand), "%s %s", m_szCommand, db);

    if (charsetName)
        dSprintf(m_szCommand, sizeof(m_szCommand), "%s CHARACTER SET ", m_szCommand, charsetName);

    if (collationName)
        dSprintf(m_szCommand, sizeof(m_szCommand), "%s %sCOLLATE %s", m_szCommand,
            (charsetName && *charsetName) ? "," : "", collationName);

    return (mysql_query(m_pMySQL, m_szCommand) == 0);
}

/*
**数据库删除
*/
bool CDBMySql::dbDrop(const char* db, bool exist)
{
    if (!db || !*db || !ping())
        return false;

    /*该函数已不再被重视，使用SQL语句DROP DATABASE取而代之。*/
    /*return (mysql_drop_db(m_pMySQL,db) == 0);*/

    memset(m_szCommand, 0, sizeof(m_szCommand));
    dSprintf(m_szCommand, sizeof(m_szCommand), "DROP DATABASE");

    if (exist)
        dSprintf(m_szCommand, sizeof(m_szCommand), "%s IF EXISTS", m_szCommand);

    dSprintf(m_szCommand, sizeof(m_szCommand), "%s %s", m_szCommand, db);

    return (mysql_query(m_pMySQL, m_szCommand) == 0);
}

/*
**数据库修改
*/
bool CDBMySql::dbAlter(const char* db, const char* charsetName, const char* collationName)
{
    if (!charsetName && !collationName)
        return false;

    if (!ping())
        return false;

    memset(m_szCommand, 0, sizeof(m_szCommand));
    dSprintf(m_szCommand, sizeof(m_szCommand), "ALTER DATABASE");

    if (db && *db)
        dSprintf(m_szCommand, sizeof(m_szCommand), "%s %s", m_szCommand, db);

    if (charsetName) /*字符集*/
        dSprintf(m_szCommand, sizeof(m_szCommand), "%s CHARACTER SET %s", m_szCommand, charsetName);

    if (collationName) /*整序*/
        dSprintf(m_szCommand, sizeof(m_szCommand), "%s %sCOLLATE %s", m_szCommand,
            (charsetName && *charsetName) ? "," : "", collationName);

    return (mysql_query(m_pMySQL, m_szCommand) == 0);
}

//-------------------------------------------------------------
//------------------------------ 表是否存在
bool CDBMySql::tableExists(const char* pTableName)
{
    if (!pTableName)
        return false;

    try {
        CMySQLMultiFree clMultiFree(this);

        this->cmd("SHOW TABLES LIKE '%s'", pTableName);
        if (!this->execute())
            return false;

        CMySQLResult clResult = this->getResult();
        if (clResult.rowMore())
            return true;
    } catch (CMySQLException& e) {
    }

    return false;
}

/*
**数据表创建
*/
bool CDBMySql::tableCreate(const char* tblName, bool temp, bool exist, const char* definition, ...)
{
    if (!tblName || !*tblName || !definition || !*definition)
        return false;

    if (!ping())
        return false;

    memset(m_szCommand, 0, sizeof(m_szCommand));
    if (temp)
        dSprintf(m_szCommand, sizeof(m_szCommand), "CREATE TEMPORARY TABLE");
    else
        dSprintf(m_szCommand, sizeof(m_szCommand), "CREATE TABLE");

    if (exist)
        dSprintf(m_szCommand, sizeof(m_szCommand), "%s IF NOT EXISTS", m_szCommand);

    dSprintf(m_szCommand, sizeof(m_szCommand), "%s %s (", m_szCommand, tblName);

    char szFormat[16 * 1024] = { 0 };
    va_list argptr;
    va_start(argptr, definition);
    dVsprintf(szFormat, sizeof(szFormat), definition, argptr);
    va_end(argptr);

    dSprintf(m_szCommand, sizeof(m_szCommand), "%s %s);", m_szCommand, szFormat);

    return (mysql_query(m_pMySQL, m_szCommand) == 0);
}

/*
**数据表删除
*/
bool CDBMySql::tableDrop(bool temp, bool exist, const char* tblName, ...)
{
    if (!tblName || !*tblName || !ping())
        return false;

    memset(m_szCommand, 0, sizeof(m_szCommand));
    if (temp)
        dSprintf(m_szCommand, sizeof(m_szCommand), "DROP TEMPORARY TABLE");
    else
        dSprintf(m_szCommand, sizeof(m_szCommand), "DROP TABLE");

    if (exist)
        dSprintf(m_szCommand, sizeof(m_szCommand), "%s IF EXISTS ", m_szCommand);

    char szFormat[16 * 1024] = { 0 };
    va_list argptr;
    va_start(argptr, tblName);
    dVsprintf(szFormat, sizeof(szFormat), tblName, argptr);
    va_end(argptr);

    dSprintf(m_szCommand, sizeof(m_szCommand), "%s %s", m_szCommand, szFormat);

    return (mysql_query(m_pMySQL, m_szCommand) == 0);
}

/*
**数据表修改
*/
bool CDBMySql::tableAlter(const char* tblName, const char* command, ...)
{
    if (!tblName || !*tblName || !command || !*command || !ping())
        return false;

    memset(m_szCommand, 0, sizeof(m_szCommand));
    dSprintf(m_szCommand, sizeof(m_szCommand), "ALTER TABLE %s", tblName);

    char szFormat[16 * 1024] = { 0 };
    va_list argptr;
    va_start(argptr, command);
    dVsprintf(szFormat, sizeof(szFormat), command, argptr);
    va_end(argptr);

    dSprintf(m_szCommand, sizeof(m_szCommand), "%s %s", m_szCommand, szFormat);

    return (mysql_query(m_pMySQL, m_szCommand) == 0);
}

/*
**数据表重命名
*/
bool CDBMySql::tableRename(const char* tblName, const char* newName)
{
    if (!tblName || !*tblName || !newName || !*newName || !ping())
        return false;

    memset(m_szCommand, 0, sizeof(m_szCommand));
    dSprintf(m_szCommand, sizeof(m_szCommand), "RENAME TABLE %s TO %s", tblName, newName);

    return (mysql_query(m_pMySQL, m_szCommand) == 0);
}

/*
**数据表重命名
*/
bool CDBMySql::tableRename(const char* tblName, ...)
{
    if (!tblName || !*tblName || !ping())
        return false;

    memset(m_szCommand, 0, sizeof(m_szCommand));
    char szFormat[16 * 1024] = { 0 };
    va_list argptr;
    va_start(argptr, tblName);
    dVsprintf(szFormat, sizeof(szFormat), tblName, argptr);
    va_end(argptr);

    dSprintf(m_szCommand, sizeof(m_szCommand), "RENAME TABLE %s", m_szCommand, szFormat);

    return (mysql_query(m_pMySQL, m_szCommand) == 0);
}

/*
**遍历数据库名
*/
CMySQLResult CDBMySql::showDatabase(const char* wild)
{
    if (!ping()) {
#ifdef _MySQL_throw
        throw CMySQLException(m_pMySQL, 0, "CDBMySql::showDatabase()");
#endif
        return CMySQLResult(m_pMySQL);
    }

    MYSQL_RES* pResult = mysql_list_dbs(m_pMySQL, wild);
    if (!pResult) {
#ifdef _MySQL_throw
        throw CMySQLException(m_pMySQL, 0, "mysql_list_dbs()");
#endif
        return CMySQLResult(m_pMySQL);
    }

    return CMySQLResult(m_pMySQL, pResult);
}

/*
**遍历当前数据库内表名
*/
CMySQLResult CDBMySql::showTables(const char* wild)
{
    if (!ping()) {
#ifdef _MySQL_throw
        throw CMySQLException(m_pMySQL, 0, "CDBMySql::showTables()");
#endif
        return CMySQLResult(m_pMySQL);
    }

    /*SHOW tables [LIKE wild]*/
    MYSQL_RES* pResult = mysql_list_tables(m_pMySQL, wild);
    if (!pResult) {
#ifdef _MySQL_throw
        throw CMySQLException(m_pMySQL, 0, "mysql_list_tables()");
#endif
        return CMySQLResult(m_pMySQL);
    }

    return CMySQLResult(m_pMySQL, pResult);
}

/*
**遍历当前数据库中表字段名
*/
CMySQLResult CDBMySql::showFields(const char* table, const char* wild)
{
    if (!ping() || !table || !*table) {
#ifdef _MySQL_throw
        throw CMySQLException(m_pMySQL, 0, "CDBMySql::showFields");
#endif
        return CMySQLResult(m_pMySQL);
    }

    MYSQL_RES* pResult = NULL;
    if (0) {
        pResult = mysql_list_fields(m_pMySQL, table, wild);
        /*SHOW COLUMNS FROM tbl_name [LIKE wild]*/ /*建议使用语句*/
        if (!pResult) {
            /*错误*/
            /*CR_COMMANDS_OUT_OF_SYNC	以不恰当的顺序执行了命令。
            CR_SERVER_GONE_ERROR		MySQL服务器不可用。
            CR_SERVER_LOST				在查询过程中，与服务器的连接丢失。
            CR_UNKNOWN_ERROR			出现未知错误。 */
        }
    } else {
        char szCmd[2 * 1024] = { 0 };
        dSprintf(szCmd, sizeof(szCmd), "SHOW COLUMNS FROM %s", table);
        if (wild)
            dSprintf(szCmd, sizeof(szCmd), "%s %s", szCmd, wild);

        if (mysql_query(m_pMySQL, szCmd) != 0) {
#ifdef _MySQL_throw
            throw CMySQLException(m_pMySQL, 0, "mysql_query()");
#endif
            return CMySQLResult(m_pMySQL);
        }

        pResult = mysql_store_result(m_pMySQL);
    }

    if (!pResult) {
#ifdef _MySQL_throw
        throw CMySQLException(m_pMySQL);
#endif
        return CMySQLResult(m_pMySQL);
    }

    return CMySQLResult(m_pMySQL, pResult);
}

/*
**格式化数据库指令
*/
ulong CDBMySql::cmd(const char* pCmd, ...)
{
    if (!pCmd)
        return 0;

    memset(m_szCommand, 0, sizeof(m_szCommand));

    va_list argptr;
    va_start(argptr, pCmd);
    m_ulCmdLength = dVsprintf(m_szCommand, sizeof(m_szCommand), pCmd, argptr);
    va_end(argptr);

    return m_ulCmdLength;
}

//-------------------------------------------------------------
//------------------------------
ulong CDBMySql::cmd_(const char* pCmd)
{
    if (!pCmd)
        return 0;

    memset(m_szCommand, 0, sizeof(m_szCommand));

    m_ulCmdLength = dStrlen(pCmd);
    if (m_ulCmdLength > sizeof(m_szCommand))
        m_ulCmdLength = sizeof(m_szCommand);

    dStrncpy(m_szCommand, sizeof(m_szCommand), pCmd, m_ulCmdLength);

    return m_ulCmdLength;
}

/*
**执行(execute)
*/
bool CDBMySql::execute(bool real)
{
    if (!m_szCommand[0] || !ping())
        return false;

    //先清理
    if (m_pMySQL) {
        MYSQL_RES* pDBResult = NULL;
        do {
            pDBResult = mysql_store_result(m_pMySQL);
            if (pDBResult)
                mysql_free_result(pDBResult);
        } while (mysql_next_result(m_pMySQL) == 0);
    }

    int nRes = 0;
    if (real)
        nRes = mysql_real_query(m_pMySQL, m_szCommand, m_ulCmdLength); /*可用于包含二进制数据的查询*/
    else
        nRes = mysql_query(m_pMySQL, m_szCommand); /*不能用于包含二进制数据的查询*/

    if (nRes != 0) {
#ifdef _MySQL_throw
        throw CMySQLException(m_pMySQL, 0, "CDBMySql::execute()");
#endif
        return false;
    }

    return true;
}

/*
**获得AUTO_INCREMENT列生成的ID
*/
my_ulonglong CDBMySql::getInsertIncrement()
{
    if (!m_pMySQL)
        return 0;

    return mysql_insert_id(m_pMySQL);
}

/*
**影响/改变行数
*/
my_ulonglong CDBMySql::getRowAffected()
{
    if (!m_pMySQL)
        return 0;

    return mysql_affected_rows(m_pMySQL);
}

/*
**获得执行结果列数(无结果集时可用)
*/
unsigned int CDBMySql::getResultFieldCount()
{
    if (!m_pMySQL)
        return 0;

    return mysql_field_count(m_pMySQL);
}

/*
**获得执行结果
*/
CMySQLResult CDBMySql::getResult()
{
    if (!m_pMySQL) {
#ifdef _MySQL_throw
        throw CMySQLException(m_pMySQL, 0, "CDBMySql::getResult()");
#endif
        return CMySQLResult(m_pMySQL);
    }
    if (!mysql_field_count(m_pMySQL)) {
#ifdef _MySQL_throw
        throw CMySQLException(m_pMySQL, 0, "mysql_field_count()");
#endif
        return CMySQLResult(m_pMySQL);
    }

    /*初始化逐行的结果集检索，直接从服务器读取结果，而不会将其保存在临时表或本地缓冲区内*/
    /*m_pDBResult = mysql_use_result(m_pMySql);*/

    /*检索完整的结果集至客户端。*/
    MYSQL_RES* pDBResult = mysql_store_result(m_pMySQL);
    if (!pDBResult) {
#ifdef _MySQL_throw
        throw CMySQLException(m_pMySQL, 0, "mysql_store_result()");
#endif
        return CMySQLResult(m_pMySQL);
    }

    return CMySQLResult(m_pMySQL, pDBResult);
}

/*
**是否存在其他结果集
*/
bool CDBMySql::resultMore()
{
    if (!m_pMySQL)
        return false;

    if (mysql_more_results(m_pMySQL) == 1)
        return false;

    /*在多语句执行过程中返回/初始化下一个结果。
    返回值0 成功并有多个结果。-1 成功但没有多个结果。 > 0 出错*/
    return (mysql_next_result(m_pMySQL) == 0);
}

/*
**获得预处理
*/
CMySQLStatement CDBMySql::createStatement()
{
    if (!ping()) {
#ifdef _MySQL_throw
        throw CMySQLException(m_pMySQL, 0, "CDBMySql::createStatement()");
#endif
        return CMySQLStatement();
    }

    //先清理
    if (m_pMySQL) {
        MYSQL_RES* pDBResult = NULL;
        do {
            pDBResult = mysql_store_result(m_pMySQL);
            if (pDBResult)
                mysql_free_result(pDBResult);
        } while (mysql_next_result(m_pMySQL) == 0);
    }
    /*创建MYSQL_STMT句柄*/
    MYSQL_STMT* pStmt = mysql_stmt_init(m_pMySQL);
    if (!pStmt) {
#ifdef _MySQL_throw
        throw CMySQLException(m_pMySQL, 0, "mysql_stmt_init()");
#endif
        return CMySQLStatement();
    }

    return CMySQLStatement(pStmt);
}
/*************************************************************
* 说明 : 多查询释放
*************************************************************/
/*
**
*/
CMySQLMultiFree::CMySQLMultiFree(MYSQL* pMySQL)
{
    m_pMySQL = pMySQL;
}

/*
**
*/
CMySQLMultiFree::CMySQLMultiFree(CDBMySql* pMySQL)
{
    if (!pMySQL)
        pMySQL = NULL;
    else
        m_pMySQL = pMySQL->m_pMySQL;
}

/*
**
*/
CMySQLMultiFree::~CMySQLMultiFree()
{
    if (m_pMySQL) {
        MYSQL_RES* pDBResult = NULL;
        do {
            pDBResult = mysql_store_result(m_pMySQL);
            if (pDBResult)
                mysql_free_result(pDBResult);
        } while (mysql_next_result(m_pMySQL) == 0);
    }
}
