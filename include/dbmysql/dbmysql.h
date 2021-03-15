/*----------------- dbMySql.h
*
* Copyright (C) 2013 
* Author: 
* Version: 1.0
* Date: 2011/8/16 21:25:06
*--------------------------------------------------------------
*MySQL链接封装,支持MySQL 1.4.1至5.1.x库版本
*------------------------------------------------------------*/
/*TODO:	MYSQL字段标志值
|	字段标志值				|	标志描述								|
|---------------------------|----------------------------|
|	NOT_NULL_FLAG			|	字段不能为NULL						|
|---------------------------|----------------------------|
|	PRI_KEY_FLAG			|	字段是主键的组成部分				|
|---------------------------|----------------------------|
|	UNIQUE_KEY_FLAG		|	字段是唯一键的组成部分			|
|---------------------------|----------------------------|
|	MULTIPLE_KEY_FLAG		|	字段是非唯一键的组成部分		|
|---------------------------|----------------------------|
|	UNSIGNED_FLAG			|	字段具有UNSIGNED属性				|
|---------------------------|----------------------------|
|	ZEROFILL_FLAG			|	字段具有ZEROFILL属性				|
|---------------------------|----------------------------|
|	BINARY_FLAG				|	字段具有BINARY属性				|
|---------------------------|----------------------------|
|	AUTO_INCREMENT_FLAG	|	字段具有AUTO_INCREMENT属性		|
|---------------------------|----------------------------|
|	ENUM_FLAG				|	字段是ENUM（不再重视）			|
|---------------------------|----------------------------|
|	SET_FLAG					|	字段是SET（不再重视）			|
|---------------------------|----------------------------|
|	BLOB_FLAG				|	字段是BLOB或TEXT（不再重视）	|
|---------------------------|----------------------------|
|	TIMESTAMP_FLAG			|	字段是TIMESTAMP（不再重视）	|
--------------------------------------------------------*/
/*TODO:	MYSQL字段类型值
|	类型值					|	类型描述				|	C类型							|	备注
|-----------------------|-----------------------------------------------|-----------------------------------|
|	MYSQL_TYPE_TINY		|	TINYINT				|	char							|	(1位)										|
|-----------------------|------------------------------|----------------------------------------------------|
|	MYSQL_TYPE_SHORT		|	SMALLINT				|	short							|	(2位)										|
|-----------------------|------------------------------|----------------------------------------------------|
|	MYSQL_TYPE_LONG		|	INTEGER				|	long/int						|	(4位)										|
|-----------------------|------------------------------|----------------------------------------------------|
|	MYSQL_TYPE_INT24		|	MEDIUMINT			|	*int							|	(24位)									|
|-----------------------|------------------------------|----------------------------------------------------|
|	MYSQL_TYPE_LONGLONG	|	BIGINT				|	long long int/__int64	|	(8位)										|
|-----------------------|------------------------------|----------------------------------------------------|
|	MYSQL_TYPE_DECIMAL	|	DECIMAL或NUMERIC	|	*float						|												|
|-----------------------|------------------------------|----------------|-----------------------------------|
|	MYSQL_TYPE_NEWDECIMAL|	DECIMAL或NUMERIC	|	*float						|	精度数学									|
|-----------------------|------------------------------|----------------|-----------------------------------|
|	MYSQL_TYPE_FLOAT		|	FLOAT					|	float							|	(4位)										|
|-----------------------|------------------------------|----------------|-----------------------------------|
|	MYSQL_TYPE_DOUBLE		|	DOUBLE或REAL		|	double						|	(8位)										|
|-----------------------|------------------------------|----------------|-----------------------------------|
|	MYSQL_TYPE_BIT			|	BIT					|	unsigned char				|	(@范围为从1到64)						|
|-----------------------|------------------------------|----------------|-----------------------------------|
|	MYSQL_TYPE_TIMESTAMP	|	TIMESTAMP			|	MYSQL_TIME					|												|
|-----------------------|--------------------|--------------------------|-----------------------------------|
|	MYSQL_TYPE_DATE		|	DATE					|	MYSQL_TIME					|												|
|-----------------------|--------------------|--------------------------|-----------------------------------|
|	MYSQL_TYPE_TIME		|	TIME					|	MYSQL_TIME					|												|
|-----------------------|------------------------------|----------------|-----------------------------------|
|	MYSQL_TYPE_DATETIME	|	DATETIME				|	MYSQL_TIME					|												|
|-----------------------|--------------------|--------------------------|-----------------------------------|
|	MYSQL_TYPE_YEAR		|	YEAR					|									|												|
|-----------------------|--------------------|--------------------------|-----------------------------------|
|	MYSQL_TYPE_STRING		|	CHAR					|									|												|
|-----------------------|--------------------|--------------------------|-----------------------------------|
|	MYSQL_TYPE_VAR_STRING|	VARCHAR				|									|												|
|-----------------------|--------------------|--------------------------|-----------------------------------|
|	MYSQL_TYPE_BLOB		|	BLOB或TEXT			|									|	（使用max_length来确定最大长度） |
|-----------------------|--------------------|--------------------------|-----------------------------------|
|	MYSQL_TYPE_SET			|	SET					|									|	(二进制值类似占第几位[可多位])	|
|-----------------------|--------------------|--------------------------|-----------------------------------|
|	MYSQL_TYPE_ENUM		|	ENUM					|									|												|
|-----------------------|--------------------|--------------------------|-----------------------------------|
|	MYSQL_TYPE_GEOMETRY	|	Spatial				|									|												|
|-----------------------|-----------------------------------------------|-----------------------------------|
|	MYSQL_TYPE_NULL		|	NULL-type			|									|												|
|-----------------------|--------------------|--------------------------|-----------------------------------|
|	MYSQL_TYPE_CHAR		|							|									|	不再重视，用MYSQL_TYPE_TINY取代	|
*----------------------------------------------------------------------------------------------------------*/
#pragma once

//#include <iostream>
#ifdef WIN32

#include <winsock2.h>
#include <windows.h>

#endif//WIN32

#include "utility/basic_functions.h"
#include "string/string_functions.h"
#include <memory.h>
#include <stdio.h>

#define HAVE_INT64
#define HAVE_UINT64

#include <mysql.h>
#include <errmsg.h>
/*************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
/*************************************************************/
#ifndef _MySQL_throw
#define _MySQL_throw
#endif //_MySQL_throw
/**************************************************************
* 说明 : MySQL异常处理
**************************************************************/
class CMySQLException
{
private:
protected:
public:
	unsigned int uCode;
	char         szMsg[512];
	char         szError[512];
	char         szSqlState[512];

public:
	CMySQLException(MYSQL* mysql, unsigned int uCode = 0, const char* pMsg = NULL);

	CMySQLException(MYSQL_STMT* stmt, unsigned int uCode = 0, const char* pMsg = NULL);

	virtual~CMySQLException()
	{
	}
};

class CMySQLResult;
class CMySQLStatement;
/*************************************************************
* 说明 : MySQL数据库链接
*************************************************************/
class CDBMySql
{
private:
protected:
	MYSQL* m_pMySQL;                /*数据库连接的句柄*/
	char         m_szCharsetName[256];    /*链接字符*/
	my_bool      m_bConnected;            /*是否已链接*/
	my_bool      m_bReConnect;            /*断开是否自动重链(默认重链接)*/
	unsigned int m_uConnectTimeout;        /*链接超时(默认3秒)*/

protected:
	ulong m_ulCmdLength;
	char  m_szCommand[16*1024];

public:
	friend class CMySQLMultiFree;

	friend class CMySQLTransaction;

public:
	inline bool isOpen()
	{
		return m_bConnected != 0;
	}

public:
	CDBMySql();

	virtual~CDBMySql();

public:
	/*--->[ 初始化数据库链接属性 ]*/
	void initialize(my_bool bReConnect = true, unsigned int uConnectTimeout = 3, const char* pCharsetName = NULL);

public:
	/*--->[ 打开链接数据库 ]
	(host=NULL或host="localhost"连接将被视为与本地主机的连接)
	(user=NULL或user=""，用户将被视为当前用户)
	(db=NULL，连接会将默认的数据库设为该值)
	(port!=0，其值将用作TCP/IP连接的端口号)
	(unix_socket!=NULL，该字符串描述了应使用的套接字或命名管道)
	*/
	bool open(const char* host, const char* user, const char* passwd, const char* db = NULL, unsigned int port = 0,
	          const char* unixSocket = NULL, unsigned long clientFlag = 0);

	/*--->[ 关闭数据库链接 ]*/
	void close();

	/*--->[ 是否链接SQL如断开会自动链接 ]
	(必须在检索出所有结果集后使用)
	*/
	bool ping();

	/*--->[ 是否启动多语句处理 ]*/
	bool multiSentence(bool bMore);

public:
	/*--->[ 设置当前链接字符集 ]
	(charsetName = "utf8",等等)
	(当输入为gbk时必须设置链接字符集为gbk,utf8同理 通用:数据库需要中文时无论字符集为gbk/utf8)
	*/
	bool setConnectCharacter(const char* charsetName);

	/*--->[ 设置当前字符集 ]*/
	bool setCharacter(const char* charsetName);

	/*--->[ 当前连接默认字符集 ]*/
	const char* getConnectCharacter();

	/*--->[ 当前连接默认字符集信息 ]*/
	bool getConnectCharacter(MY_CHARSET_INFO& sInfo);

public:
	bool getServerInfo();

	bool getClientInfo();

	/*--->[ 显示当前服务器线程的列表 ]*/
	CMySQLResult showProcessesList();

	/*--->[ 获得当前连接的线程ID ]*/
	unsigned long getConnectThreadID();

	/*--->[ 获得最后一次SQL语句生成的告警数 ]*/
	unsigned int getLastWarningCount();

	/*--->[ 获得执行查询的字符串信息 ]*/
	const char* getExecuteInfo();

public:
	/*--->[ 更改连接上的用户和数据库 ]*/
	bool changeConnectUser(const char* user, const char* passwd, const char* db);

	/*--->[ 数据库选择 ]*/
	bool dbSelect(const char* db);

	/*--->[ 数据库创建 ]
	(db = 数据库名)
	(charsetName = 字符集名)
	(collationName = 整序)
	(exist = true 不存在时才创建)
	*/
	bool dbCreate(const char* db, const char* charsetName = NULL, const char* collationName = NULL,
	              bool exist = true);

	/*--->[ 数据库删除 ]*/
	bool dbDrop(const char* db, bool exist = true);

	/*--->[ 数据库修改 ]
	(db = 数据库名)
	(charsetName = 字符集名)
	(collationName = 整序)
	*/
	bool dbAlter(const char* db, const char* charsetName, const char* collationName);

public:
	/*----->{ 表是否存在 }*/
	bool tableExists(const char* pTableName);

	/*--->[ 数据表创建 ]
	(tblName = 表名)
	(temp = 是否临时表)
	(exist = true 不存在时才创建)
	(definition = 定义)
	*/
	bool tableCreate(const char* tblName, bool temp, bool exist, const char* definition, ...);

	/*--->[ 数据表删除 ]*/
	bool tableDrop(bool temp, bool exist, const char* tblName, ...);

	/*--->[ 数据表修改 ]*/ /*ADD | ALTER | CHANGE | MODIFY | DROP*/
	bool tableAlter(const char* tblName, const char* command, ...);

	/*--->[ 数据表重命名 ]*/
	bool tableRename(const char* tblName, const char* newName);

	/*--->[ 数据表重命名 ]*/
	bool tableRename(const char* tblName, ...);

public:
	/*--->[ 遍历数据库名 ]
	(wild = 简单正则表达式条件,可包含通配符“%”或“_”)
	*/
	CMySQLResult showDatabase(const char* wild = NULL);

	/*--->[ 遍历当前数据库内表名 ]
	(wild = 简单正则表达式条件,可包含通配符“%”或“_”)
	*/
	CMySQLResult showTables(const char* wild = NULL);

	/*--->[ 遍历当前数据库中表字段名 ]
	(wild = 简单正则表达式条件,可包含通配符“%”或“_”)
	*/
	CMySQLResult showFields(const char* table, const char* wild = NULL);

public:
	/*--->[ 格式化数据库指令 ]
	(不应为语句添加终结分号（‘;’）或“\g”)
	*/
	ulong cmd(const char* pCmd, ...);

	ulong cmd_(const char* pCmd);

	/*--->[ 执行(execute) ]*/
	bool execute(bool real = true);

	/*--->[ 获得AUTO_INCREMENT列生成的ID ]*/
	my_ulonglong getInsertIncrement();

	/*--->[ 获得影响/改变行数 ]*/
	my_ulonglong getRowAffected();

	/*--->[ 获得执行结果列数(无结果集时可用) ]*/
	unsigned int getResultFieldCount();

	/*--->[ 获得执行结果 ]*/
	CMySQLResult getResult();

public:
	/*--->[ 下一个结果集 ]
	(可使用do{}while(resultMore());)
	*/
	bool resultMore();

public:
	/*--->[ 获得预处理 ]*/
	CMySQLStatement createStatement();
};
/*************************************************************
* 说明 : 多查询释放
*	如果使用多语句查询或存储过程查询时必须调用
*************************************************************/
class CMySQLMultiFree
{
private:
protected:
	MYSQL* m_pMySQL;            /*数据库连接的句柄*/
public:
	CMySQLMultiFree(MYSQL* pMySQL);

	CMySQLMultiFree(CDBMySql* pMySQL);

	virtual~CMySQLMultiFree();
};

/*************************************************************
* 说明 : MySQL事务
*************************************************************/
class CMySQLTransaction
{
private:
protected:
	MYSQL* m_pMySQL;
	bool m_bEndTrans;

public:
	CMySQLTransaction(MYSQL* mysql);

	CMySQLTransaction(CDBMySql* pMySQL);

	virtual ~CMySQLTransaction();

public:
	/*--->[ 开始事务 ]*/
	bool begin();

	/*--->[ 提交事务 ]*/
	bool commit();

	/*--->[ 回滚事务 ]*/
	bool rollback();
};

/*************************************************************
* 说明 : 数据库查询结果集
*************************************************************/
class CMySQLResult
{
protected:
	MYSQL    * m_pMySQL;            /*数据库连接的句柄(主要用于异常报错)*/
	MYSQL_RES* m_pResult;            /*返回行的查询结果集*/
	MYSQL_ROW    m_pRow;                /*这是1行数据*/
	my_ulonglong m_ulRowCount;        /*行数量*/
	unsigned int m_uFiledCount;        /*列数量*/
	unsigned long* m_pulFiledsLength;    /*列数据长度*/

public:
	CMySQLResult(MYSQL* mysql = NULL, MYSQL_RES* pResult = NULL);

	virtual~CMySQLResult();

public:
	inline MYSQL_RES* getMYSQL_RES()
	{
		return m_pResult;
	}

	inline MYSQL_ROW getMYSQL_ROW()
	{
		return m_pRow;
	}

public:
	/*--->[ 获得结果集行数 ]*/
	inline my_ulonglong getRowCount()
	{
		return m_ulRowCount;
	}

	/*--->[ 获得结果集列数 ]*/
	inline unsigned int getFiledCount()
	{
		return m_uFiledCount;
	}

	/*--->[ 获得结果集列数据长度 ]*/
	inline unsigned long* getFiledsLength()
	{
		return m_pulFiledsLength;
	}

public:
	CMySQLResult& operator=(CMySQLResult& clMySQLResult);

public:
	/*--->[ 初始化结果集 ]*/
	void initialize(MYSQL_RES* pResult);

	/*--->[ 释放结果集使用的内存 ]*/
	void freeResult();

public:
	/*--->[ 返回上次字段光标的位置 ]
	(返回上次所使用字段光标的位置)
	*/
	MYSQL_FIELD_OFFSET fieldTell();

	/*--->[ 将列光标置于指定的列 ]
	(开始为0到FieldCount)
	*/
	MYSQL_FIELD_OFFSET fieldSeek(MYSQL_FIELD_OFFSET offset);

	/*--->[ 检索关于结果集中所有列的信息(下一个字段) ]*/
	MYSQL_FIELD* fieldFetch();

	/*--->[ 结果集内某列的信息 ]
	(fieldnr = 0到FieldCount-1)
	*/
	MYSQL_FIELD* fieldFetchDirect(unsigned int fieldnr);

	/*--->[ 返回所有字段结构的数组 ]
	(取结果集[0到FieldCount-1].name)
	*/
	MYSQL_FIELD* fieldFetchs();

	/*--->[ 获得列数据长度 ]*/
	unsigned long getFiledDataLength(unsigned int uField);

	/*--->[ 获得列名 ]*/
	const char* getFiledName(unsigned int uField)
	{
		MYSQL_FIELD* pField = fieldFetchDirect(uField);
		return ((pField != NULL) ? pField->name : NULL);
	}

	/*--->[ 获得列名长度 ]*/
	unsigned int getFiledNameLength(unsigned int uField)
	{
		MYSQL_FIELD* pField = fieldFetchDirect(uField);
		return ((pField != NULL) ? pField->name_length : 0);
	}

	/*--->[ 获得列宽度 ]*/
	unsigned long getFiledLength(unsigned int uField)
	{
		MYSQL_FIELD* pField = fieldFetchDirect(uField);
		return ((pField != NULL) ? pField->length : 0);
	}

	/*--->[ 获得列最大宽度 ]*/
	unsigned long getFiledMaxLength(unsigned int uField)
	{
		MYSQL_FIELD* pField = fieldFetchDirect(uField);
		return ((pField != NULL) ? pField->max_length : 0);
	}

	/*--->[ 获得列位标志(文件头) ]
	(IS_NOT_NULL(flags)	|	如果该字段定义为NOT NULL，为“真”)
	(IS_PRI_KEY(flags)	|	如果该字段是主键，为“真”)
	(IS_BLOB(flags)		|	如果该字段是BLOB或TEXT，为“真”（不再重视，用测试field->type取而代之)
	*/
	unsigned int getFiledFlags(unsigned int uField)
	{
		MYSQL_FIELD* pField = fieldFetchDirect(uField);
		return ((pField != NULL) ? pField->flags : 0);
	}

	/*--->[ 获得列类型(文件头) ]*/
	enum_field_types getFiledType(unsigned int uField)
	{
		MYSQL_FIELD* pField = fieldFetchDirect(uField);
		return ((pField != NULL) ? pField->type : MYSQL_TYPE_DECIMAL);
	}

	//---
	const char* getFiledDefaultValue(unsigned int uField)
	{
		MYSQL_FIELD* pField = fieldFetchDirect(uField);
		return ((pField != NULL) ? pField->def : NULL);
	}

public:
	/*--->[ 存在行数据 ]
	(while(rowMore()){})
	*/
	bool rowMore();

	/*--->[ 是否已到结果行尾 ]*/
	bool rowEof();

	/*--->[ 取出第几行数据 ]
	(offset = 0到RowCount-1)
	*/
	bool dataSeek(my_ulonglong offset);

	/*--->[ 当前行光标 ]*/
	MYSQL_ROWS* rowTell();

	/*--->[ 移动行光标 ]*/
	MYSQL_ROWS* rowSeek(MYSQL_ROWS* offset);

public:
	/*--->[ 获取当前行字段数据/字符串数据 ]*/
	const char* getData(unsigned int uField, const char* pDefault = NULL);

	/*--->[ 获得布尔字段数据 ]*/
	inline bool getBool(unsigned int uField, bool bDefault = false)
	{
		const char* pValue = getData(uField);
		return (pValue ? static_cast<bool>(strtol(pValue, NULL, 10) > 0) : bDefault);
	}

	/*--->[ 获得浮点字段数据 ]*/
	inline float getFloat(unsigned int uField, float fDefault = 0.0f)
	{
		const char* pValue = getData(uField);
		return (pValue ? static_cast<float>(strtof(pValue, NULL)) : fDefault);
	}

	/*--->[ 获得双精度字段数据 ]*/
	inline double getDouble(unsigned int uField, double dDefault = 0)
	{
		const char* pValue = getData(uField);
		return (pValue ? static_cast<double>(strtod(pValue, NULL)) : dDefault);
	}

	/*--->[ 获得8位整型字段数据 ]*/
	inline int8_t getInt8(unsigned int uField, int8_t iDefault = 0)
	{
		const char* pValue = getData(uField);
		return (pValue ? static_cast<int8_t>(strtol(pValue, NULL, 10)) : iDefault);
	}

	/*--->[ 获得8位无符号整型字段数据 ]*/
	inline uint8_t getUInt8(unsigned int uField, uint8_t uDefault = 0)
	{
		const char* pValue = getData(uField);
		return (pValue ? static_cast<uint8_t>(strtol(pValue, NULL, 10)) : uDefault);
	}

	/*--->[ 获得16位短整型字段数据 ]*/
	inline int16_t getInt16(unsigned int uField, int16_t iDefault = 0)
	{
		const char* pValue = getData(uField);
		return (pValue ? static_cast<int16_t>(strtol(pValue, NULL, 10)) : iDefault);
	}

	/*--->[ 获得16位无符号短整型字段数据 ]*/
	inline uint16_t getUInt16(unsigned int uField, uint16_t uDefault = 0)
	{
		const char* pValue = getData(uField);
		return (pValue ? static_cast<uint16_t>(strtol(pValue, NULL, 10)) : uDefault);
	}

	/*--->[ 获得32位整型字段数据 ]*/
	inline int32_t getInt32(unsigned int uField, int32_t iDefault = 0)
	{
		const char* pValue = getData(uField);
		return (pValue ? static_cast<int32_t>(strtol(pValue, NULL, 10)) : iDefault);
	}

	/*--->[ 获得32位无符号整型字段数据 ]*/
	inline uint32_t getUInt32(unsigned int uField, uint32_t uDefault = 0)
	{
		const char* pValue = getData(uField);
		return (pValue ? static_cast<uint32_t>(strtol(pValue, NULL, 10)) : uDefault);
	}

	/*--->[ 获得64位整型字段数据 ]*/
	inline int64_t getInt64(unsigned int uField, int64_t iDefault = 0)
	{
		const char* pValue = getData(uField);
		return (pValue ? strtoll(pValue, NULL, 10) : iDefault);
	}

	/*--->[ 获得64位无符号整型字段数据 ]*/
	inline uint64_t getUInt64(unsigned int uField, uint64_t uDefault = 0)
	{
		const char* pValue = getData(uField);
		return (pValue ? strtoull(pValue, NULL, 10) : uDefault);
	}
};

/*************************************************************
* 说明 : 语句预处理
* 用法 :*在进行语句预处理时想要进行其他查询必须close()掉
*		1、调用cmd()预处理语句返回可绑定参数数量
		2、调用Bind...()绑定各参数数据缓冲
		3、设置绑定参数函数数据值
		4、调用execute()执行语句，如需再次执行结果可重复3、4条
		5、调用getRowAffected()查看影响行数
*************************************************************/
class CMySQLStatement
{
private:
protected:
	MYSQL_STMT* m_pStatement;    /*预处理语句句柄*/

protected:
	MYSQL_BIND   * m_pParamBind;    /*需绑定参数数据*/
	unsigned long* m_pParamLength;    /*参数缓冲区长度*/
	unsigned long m_ulParamCount;    /*需绑定参数数量*/

protected:
	my_ulonglong m_ulRowCount;    /*结果行数量*/
	MYSQL_BIND* m_pResultBind;    /*需绑定结果列数据*/
	unsigned long m_ulResultCount;/*需绑定结果列数量*/

public:
	CMySQLStatement(MYSQL_STMT* pStatement = NULL);

	virtual~CMySQLStatement();

public:
	/*--->[ 获得预处理句柄 ]*/
	inline MYSQL_STMT* getMYSQL_STMT()
	{
		return m_pStatement;
	}

	/*--->[ 获得绑定参数数量 ]*/
	inline unsigned long getParamCount()
	{
		return m_ulParamCount;
	}

	/*--->[ 获得绑定结果列数量 ]*/
	inline unsigned long getResultCount()
	{
		return m_ulResultCount;
	}

public:
	CMySQLStatement& operator=(CMySQLStatement& clMySQLStatement);

protected:
	/*--->[ 释放绑定缓冲 ]*/
	void freeBind();

public:
	/*--->[ 关闭预处理语句 ]*/
	bool close();

	/*--->[ 释放与执行预处理语句生成的结果集有关的内存 ]*/
	bool freeResult();

public:
	/*--->[ 格式化数据库指令并准备 ]
	(不应为语句添加终结分号（‘;’）或“\g”)
	*/
	bool cmd(const char* pCmd, ...);

	/*--->[ 执行(execute) ]
	(非查询在执行之前绑定所有参数,必须调用bindParams())
	(填写所有参数数据+sendLongData()完成)
	(每调用一次执行一次结果)
	*/
	bool execute();

	/*--->[ 预处理语句复位为完成准备后的状态 ]
	(撤销发送长数据到服务器)
	*/
	bool reset();

	/*--->[ 获得影响/改变行数 ]*/
	my_ulonglong getRowAffected();

	/*--->[ 获得AUTO_INCREMENT列生成的ID ]*/
	my_ulonglong getInsertIncrement();

public:
	/*--->[ 绑定参数 ]
	(完成所有绑定参数后调用结束本次绑定)
	*/
	bool bindParams();

	/*--->[ 发送长数据到服务器 ]
	(必须先绑定参数bindParams(),可对TEXT或BLOB数据类型字段进行多次数据发送填充)
	(parameter_number = 参数编号,从0开始)
	(data,length = 数据缓冲区及本次长度)
	*/
	bool sendLongData(unsigned int parameter_number, const char* data, unsigned long length);

public:
	/*--->[ 绑定结果集 ]
	(可在执行后调用)
	(完成所有绑定结果后调用结束本次绑定)
	*/
	bool bindResults();

	/*--->[ 是否存在结果 ]
	(while(rowMore()){})
	*/
	bool rowMore();

	/*--->[ 取出第几行数据 ]
	(offset = 0到RowCount-1)
	*/
	bool dataSeek(my_ulonglong offset);

	/*--->[ 从当前结果集行获取1列 ]
	(column = 列编号,从0开始)
	(offset = 数据值内的偏移量，将从该处开始检索数据。)
	*/
	bool getColume(MYSQL_BIND* bind, unsigned int column, unsigned long offset = 0);

public:/*绑定传入参数在执行前*/
	/*--->[ 绑定参数 ]*/
	bool bindParam(unsigned int uField, enum_field_types buffer_type, void* buffer, unsigned long length,
	               my_bool* is_null = NULL);

	/*--->[ 绑定位字段数据 ]*/
	inline bool bindParam_Bit(unsigned int uField, uint8_t* buffer, my_bool* is_null = NULL)
	{
		return bindParam(uField, MYSQL_TYPE_BIT, buffer, 0, is_null);
	}

	/*--->[ 绑定日期时间字段数据 ]*/
	inline bool bindParam_DateTime(unsigned int uField, MYSQL_TIME* buffer, my_bool* is_null = NULL)
	{
		return bindParam(uField, MYSQL_TYPE_DATETIME, buffer, 0, is_null);
	}

	/*--->[ 绑定字符串字段数据 ]*/
	inline bool bindParam_String(unsigned int uField, char* buffer, unsigned long length, my_bool* is_null = NULL)
	{
		return bindParam(uField, MYSQL_TYPE_STRING, buffer, length, is_null);
	}

	/*--->[ 绑定二进制字段数据 ]*/
	inline bool bindParam_Blob(unsigned int uField, void* buffer, unsigned long length, my_bool* is_null = NULL)
	{
		return bindParam(uField, MYSQL_TYPE_BLOB, buffer, length, is_null);
	}

	/*--->[ 绑定整型参数 ]*/
	bool bindParam_Int(unsigned int uField, enum_field_types buffer_type, my_bool is_unsigned, void* buffer,
	                   my_bool* is_null = NULL);

	/*--->[ 绑定布尔字段数据 ]*/
	inline bool bindParam_Bool(unsigned int uField, bool* buffer, my_bool* is_null = NULL)
	{
		return bindParam_Int(uField, MYSQL_TYPE_TINY, true, buffer, is_null);
	}

	/*--->[ 绑定浮点字段数据 ]*/
	inline bool bindParam_Float(unsigned int uField, float* buffer, my_bool* is_null = NULL)
	{
		return bindParam_Int(uField, MYSQL_TYPE_FLOAT, false, buffer, is_null);
	}

	/*--->[ 绑定双精度字段数据 ]*/
	inline bool bindParam_Double(unsigned int uField, double* buffer, my_bool* is_null = NULL)
	{
		return bindParam_Int(uField, MYSQL_TYPE_DOUBLE, false, buffer, is_null);
	}

	/*--->[ 绑定8位有符号整数字段数据 ]*/
	inline bool bindParam_Int8(unsigned int uField, int8_t* buffer, my_bool* is_null = NULL)
	{
		return bindParam_Int(uField, MYSQL_TYPE_TINY, false, buffer, is_null);
	}

	/*--->[ 绑定8位无符号整数字段数据 ]*/
	inline bool bindParam_UInt8(unsigned int uField, uint8_t* buffer, my_bool* is_null = NULL)
	{
		return bindParam_Int(uField, MYSQL_TYPE_TINY, true, buffer, is_null);
	}

	/*--->[ 绑定16位有符号整数字段数据 ]*/
	inline bool bindParam_Int16(unsigned int uField, int16_t* buffer, my_bool* is_null = NULL)
	{
		return bindParam_Int(uField, MYSQL_TYPE_SHORT, false, buffer, is_null);
	}

	/*--->[ 绑定16位无符号整数字段数据 ]*/
	inline bool bindParam_UInt16(unsigned int uField, uint16_t* buffer, my_bool* is_null = NULL)
	{
		return bindParam_Int(uField, MYSQL_TYPE_SHORT, true, buffer, is_null);
	}

	/*--->[ 绑定32位有符号整数字段数据 ]*/
	inline bool bindParam_Int32(unsigned int uField, int32_t* buffer, my_bool* is_null = NULL)
	{
		return bindParam_Int(uField, MYSQL_TYPE_LONG, false, buffer, is_null);
	}

	/*--->[ 绑定32位无符号整数字段数据 ]*/
	inline bool bindParam_UInt32(unsigned int uField, uint32_t* buffer, my_bool* is_null = NULL)
	{
		return bindParam_Int(uField, MYSQL_TYPE_LONG, true, buffer, is_null);
	}

	/*--->[ 绑定64位有符号整数字段数据 ]*/
	inline bool bindParam_Int64(unsigned int uField, int64_t* buffer, my_bool* is_null = NULL)
	{
		return bindParam_Int(uField, MYSQL_TYPE_LONGLONG, false, buffer, is_null);
	}

	/*--->[ 绑定64位无符号整数字段数据 ]*/
	inline bool bindParam_UInt64(unsigned int uField, uint64_t* buffer, my_bool* is_null = NULL)
	{
		return bindParam_Int(uField, MYSQL_TYPE_LONGLONG, true, buffer, is_null);
	}

public:/*绑定传入参数可在执行后*/
	/*--->[ 绑定结果 ]*/
	bool bindResult(unsigned int uField, enum_field_types buffer_type, void* buffer, unsigned long buffer_length,
	                my_bool* is_null = NULL);

	/*--->[ 绑定结果位字段数据 ]*/
	inline bool bindResult_Bit(unsigned int uField, uint8_t* buffer, my_bool* is_null = NULL)
	{
		return bindResult(uField, MYSQL_TYPE_BIT, buffer, 0, is_null);
	}

	/*--->[ 绑定结果日期时间字段数据 ]*/
	inline bool bindResult_DateTime(unsigned int uField, MYSQL_TIME* buffer, my_bool* is_null = NULL)
	{
		return bindResult(uField, MYSQL_TYPE_DATETIME, buffer, 0, is_null);
	}

	/*--->[ 绑定结果字符串字段数据 ]*/
	inline bool bindResult_String(unsigned int uField, char* buffer, unsigned long buffer_length,
	                              my_bool* is_null = NULL)
	{
		return bindResult(uField, MYSQL_TYPE_STRING, buffer, buffer_length, is_null);
	}

	/*--->[ 绑定结果二进制字段数据 ]*/
	inline bool bindResult_Blob(unsigned int uField, void* buffer, unsigned long buffer_length,
	                            my_bool* is_null = NULL)
	{
		return bindResult(uField, MYSQL_TYPE_BLOB, buffer, buffer_length, is_null);
	}

	/*--->[ 绑定结果整型 ]*/
	bool bindResult_Int(unsigned int uField, enum_field_types buffer_type, my_bool is_unsigned, void* buffer,
	                    my_bool* is_null = NULL);

	/*--->[ 绑定结果布尔字段数据 ]*/
	inline bool bindResult_Bool(unsigned int uField, bool* buffer, my_bool* is_null = NULL)
	{
		return bindResult_Int(uField, MYSQL_TYPE_TINY, true, buffer, is_null);
	}

	/*--->[ 绑定结果浮点字段数据 ]*/
	inline bool bindResult_Float(unsigned int uField, float* buffer, my_bool* is_null = NULL)
	{
		return bindResult_Int(uField, MYSQL_TYPE_FLOAT, false, buffer, is_null);
	}

	/*--->[ 绑定结果双精度字段数据 ]*/
	inline bool bindResult_Double(unsigned int uField, double* buffer, my_bool* is_null = NULL)
	{
		return bindResult_Int(uField, MYSQL_TYPE_DOUBLE, false, buffer, is_null);
	}

	/*--->[ 绑定结果8位有符号整数字段数据 ]*/
	inline bool bindResult_Int8(unsigned int uField, int8_t* buffer, my_bool* is_null = NULL)
	{
		return bindResult_Int(uField, MYSQL_TYPE_TINY, false, buffer, is_null);
	}

	/*--->[ 绑定结果8位无符号整数字段数据 ]*/
	inline bool bindResult_UInt8(unsigned int uField, uint8_t* buffer, my_bool* is_null = NULL)
	{
		return bindResult_Int(uField, MYSQL_TYPE_TINY, true, buffer, is_null);
	}

	/*--->[ 绑定结果16位有符号整数字段数据 ]*/
	inline bool bindResult_Int16(unsigned int uField, int16_t* buffer, my_bool* is_null = NULL)
	{
		return bindResult_Int(uField, MYSQL_TYPE_SHORT, false, buffer, is_null);
	}

	/*--->[ 绑定结果16位无符号整数字段数据 ]*/
	inline bool bindResult_UInt16(unsigned int uField, uint16_t* buffer, my_bool* is_null = NULL)
	{
		return bindResult_Int(uField, MYSQL_TYPE_SHORT, true, buffer, is_null);
	}

	/*--->[ 绑定结果32位有符号整数字段数据 ]*/
	inline bool bindResult_Int32(unsigned int uField, int32_t* buffer, my_bool* is_null = NULL)
	{
		return bindResult_Int(uField, MYSQL_TYPE_LONG, false, buffer, is_null);
	}

	/*--->[ 绑定结果32位无符号整数字段数据 ]*/
	inline bool bindResult_UInt32(unsigned int uField, uint32_t* buffer, my_bool* is_null = NULL)
	{
		return bindResult_Int(uField, MYSQL_TYPE_LONG, true, buffer, is_null);
	}

	/*--->[ 绑定结果64位有符号整数字段数据 ]*/
	inline bool bindResult_Int64(unsigned int uField, int64_t* buffer, my_bool* is_null = NULL)
	{
		return bindResult_Int(uField, MYSQL_TYPE_LONGLONG, false, buffer, is_null);
	}

	/*--->[ 绑定结果64位无符号整数字段数据 ]*/
	inline bool bindResult_UInt64(unsigned int uField, uint64_t* buffer, my_bool* is_null = NULL)
	{
		return bindResult_Int(uField, MYSQL_TYPE_LONGLONG, true, buffer, is_null);
	}
};
//****************************************************************
#ifdef __cplusplus
}
#endif



