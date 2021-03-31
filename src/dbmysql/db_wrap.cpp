#include "dbmysql/db_wrap.h"
#include "svrlib.h"

using namespace std;
using namespace svrlib;

CDBWrap::CDBWrap() {
    m_pTransaction = nullptr;
    m_debugLog = true;
}

CDBWrap::~CDBWrap() {
    dbClose();
}

bool CDBWrap::dbOpen(string host, string user, string passwd, string db, unsigned int port) {
    try {
        return m_clDatabase.open(host.c_str(), user.c_str(), passwd.c_str(), db.c_str(), port);
    }
    catch (CMySQLException &e) {
        LOG_ERROR("CDBOperator::dbOpen[host={},[port]={},user = {},pwd = {}]:{}({})->{}->{}", host, port, user, passwd,
                  e.uCode,
                  e.szError, e.szMsg, e.szSqlState);
    }

    return false;
}

bool CDBWrap::dbSelect(string db) {
    try {
        if (!m_clDatabase.dbSelect(db.c_str())) {
            LOG_ERROR("change db fail:{}", db);
            return false;
        }
        return true;
    }
    catch (CMySQLException &e) {
        LOG_ERROR("{}({})->{}->{}", e.uCode, e.szError, e.szMsg, e.szSqlState);
        return false;
    }
    return true;
}

//------------------------------
void CDBWrap::dbClose() {
    m_clDatabase.close();
}

bool CDBWrap::ping() {
    try {
        if (!m_clDatabase.ping()) {
            LOG_ERROR("ping db fail");
            return false;
        }
        return true;
    }
    catch (CMySQLException &e) {
        LOG_ERROR("{}({})->{}->{}", e.uCode, e.szError, e.szMsg, e.szSqlState);
        return false;
    }
    return false;
}

/*--->[ 开始事务 ]*/
bool CDBWrap::begin() {
    m_pTransaction = make_shared<CMySQLTransaction>(&m_clDatabase);
    return m_pTransaction->begin();
}

/*--->[ 提交事务 ]*/
bool CDBWrap::commit() {
    if (m_pTransaction) {
        return m_pTransaction->commit();
    }
    return false;
}

/*--->[ 回滚事务 ]*/
bool CDBWrap::rollback() {
    if (m_pTransaction) {
        return m_pTransaction->rollback();
    }
    return false;
}

bool CDBWrap::ExeSql(const string &strSql) {
    if (strSql.length() > MAX_SQL_LEN) {
        LOG_ERROR("the sql is too length:{}", strSql.length());
        return false;
    }
    try {
        CMySQLMultiFree clMultiFree(&m_clDatabase);
        CMySQLStatement clStatement = m_clDatabase.createStatement();
        clStatement.cmd(strSql.c_str());
        if (!clStatement.execute()) {
            LOG_ERROR("sql:{} execute fail!", strSql.c_str());
            return false;
        }
        DebugSqlLog(strSql);
    }
    catch (CMySQLException &e) {
        LOG_ERROR("{}({})->{}->{}-sql:{}", e.uCode, e.szError, e.szMsg, e.szSqlState, strSql.c_str());
        return false;
    }
    return true;
}

// 获得sql 语句的结果行数
int CDBWrap::GetResNumExeSql(const string &strSql) {
    if (strSql.length() > MAX_SQL_LEN) {
        LOG_ERROR("the sql is too length:{}", strSql.length());
        return -1;
    }
    try {
        CMySQLMultiFree clMultiFree(&m_clDatabase);
        m_clDatabase.cmd(strSql.c_str());
        if (!m_clDatabase.execute()) {
            LOG_ERROR("execute fail!,{}", strSql);
            return -1;
        }
        DebugSqlLog(strSql);
        CMySQLResult clResult = m_clDatabase.getResult();
        return clResult.getRowCount();
    }
    catch (CMySQLException &e) {
        LOG_ERROR("Error:{}({})->{}->{}->sql:{}", e.uCode, e.szError, e.szMsg, e.szSqlState, strSql);
        return -1;
    }
    return 0;
}

// 获得影响行数
int CDBWrap::GetAffectedNumExeSql(const string &strSql) {
    if (strSql.length() > MAX_SQL_LEN) {
        LOG_ERROR("the sql is too length:{}", strSql.length());
        return -1;
    }
    try {
        CMySQLMultiFree clMultiFree(&m_clDatabase);
        m_clDatabase.cmd(strSql.c_str());
        if (!m_clDatabase.execute()) {
            LOG_ERROR("execute fail!,{}", strSql);
            return -1;
        }
        DebugSqlLog(strSql);
        return m_clDatabase.getRowAffected();
    }
    catch (CMySQLException &e) {
        LOG_ERROR("Error:{}({})->{}->{}->sql:{}", e.uCode, e.szError, e.szMsg, e.szSqlState, strSql);
        return -1;
    }
    return 0;
}

int CDBWrap::Query(const string &strSql, vector<CMysqlResultRow> &vecData, int64_t &affectRow) {
    try {
        CMySQLMultiFree clMultiFree(&m_clDatabase);
        m_clDatabase.cmd(strSql.c_str());
        if (!m_clDatabase.execute()) {
            LOG_ERROR("mysql execute fail!:{}", strSql);
            return -1;
        }
        DebugSqlLog(strSql);
        CMySQLResult clResult = m_clDatabase.getResult();
        while (clResult.rowMore()) {
            m_tmpRowData.Clear();
            for (uint32_t i = 0; i < clResult.getFiledCount(); i++) {
                m_tmpRowData.SetData(clResult.getFiledName(i), (char *) clResult.getData(i),
                                     clResult.getFiledDataLength(i));
            }
            vecData.push_back(m_tmpRowData);
        }
        affectRow = m_clDatabase.getRowAffected();
        return 0;
    }
    catch (CMySQLException &e) {
        LOG_ERROR("Error:{}({})->{}->{}->sql:{}", e.uCode, e.szError, e.szMsg, e.szSqlState, strSql);
    }
    return 0;
}

bool CDBWrap::ExeBindSql(const string &strSql, vector<CBindParam> &bindParams) {
    try {
        CMySQLMultiFree clMultiFree(&m_clDatabase);
        CMySQLStatement clStatement = m_clDatabase.createStatement();
        if (!clStatement.cmd(strSql.c_str())) {
            LOG_ERROR("mysql cmd format fail!:{}", strSql);
            return false;
        }

        for (uint16_t nBind = 0; nBind < bindParams.size(); ++nBind) {
            MYSQLValue &param = bindParams[nBind];
            uint8_t valType = param.DataType();
            switch (valType) {
                case MYSQLValue::emTYPE_BLOB: {
                    clStatement.bindParam_Blob(nBind, (void *) param.Data(), param.Size());
                    break;
                }
                case MYSQLValue::emTYPE_STR: {
                    clStatement.bindParam_String(nBind, (char *) param.Data(), param.Size());
                    break;
                }
                case MYSQLValue::emTYPE_INT8: {
                    clStatement.bindParam_Int8(nBind, (int8_t *) param.Data());
                    break;
                }
                case MYSQLValue::emTYPE_UINT8: {
                    clStatement.bindParam_UInt8(nBind, (uint8_t *) param.Data());
                    break;
                }
                case MYSQLValue::emTYPE_INT16: {
                    clStatement.bindParam_Int16(nBind, (int16_t *) param.Data());
                    break;
                }
                case MYSQLValue::emTYPE_UINT16: {
                    clStatement.bindParam_UInt16(nBind, (uint16_t *) param.Data());
                    break;
                }
                case MYSQLValue::emTYPE_INT32: {
                    clStatement.bindParam_Int32(nBind, (int32_t *) param.Data());
                    break;
                }
                case MYSQLValue::emTYPE_UINT32: {
                    clStatement.bindParam_UInt32(nBind, (uint32_t *) param.Data());
                    break;
                }
                case MYSQLValue::emTYPE_INT64: {
                    clStatement.bindParam_Int64(nBind, (int64_t *) param.Data());
                    break;
                }
                case MYSQLValue::emTYPE_UINT64: {
                    clStatement.bindParam_UInt64(nBind, (uint64_t *) param.Data());
                    break;
                }
                default: {
                    LOG_ERROR("mysql bindparams type is error {}", valType);
                }

            }
        }
        clStatement.bindParams();

        if (!clStatement.execute()) {
            LOG_ERROR("mysql execute bind params fail!");
            return false;
        }
        DebugSqlLog(strSql);
        return true;
    }
    catch (CMySQLException &e) {
        LOG_ERROR("Error:{}({})->{}->{}->sql:{}", e.uCode, e.szError, e.szMsg, e.szSqlState, strSql);
    }

    return false;
}

// 插入数据
int CDBWrap::Insert(string tblName, SQLJoin &data) {
    string strSql = GetInsertSql(tblName, data);
    try {
        CMySQLMultiFree clMultiFree(&m_clDatabase);
        m_clDatabase.cmd(strSql.c_str());
        if (!m_clDatabase.execute()) {
            LOG_ERROR("execute fail!,{}", strSql);
            return -1;
        }
        DebugSqlLog(strSql);
        return m_clDatabase.getInsertIncrement();
    }
    catch (CMySQLException &e) {
        LOG_ERROR("Error:{}({})->{}->{}->sql:{}", e.uCode, e.szError, e.szMsg, e.szSqlState, strSql);
        return -1;
    }
    return 0;
}

// 更新数据
int CDBWrap::Update(string tblName, SQLJoin &data, SQLJoin &where) {
    return GetAffectedNumExeSql(GetUpdateSql(tblName, data, where));
}

// 插入或者更新
int CDBWrap::UpdateOrInsert(string tblName, SQLJoin &data) {
    return GetAffectedNumExeSql(GetUpdateOrInsertSql(tblName, data));
}

// 删除数据
int CDBWrap::Delete(string tblName, SQLJoin &where) {
    return GetAffectedNumExeSql(GetDeleteSql(tblName, where));
}

// 插入数据
string CDBWrap::GetInsertSql(string tblName, SQLJoin &data) {
    stringstream ss;
    ss << "insert into " << tblName << "( "
       << data.keys()
       << " ) values( "
       << data.values()
       << " );";
    return ss.str().c_str();
}

// 更新数据
string CDBWrap::GetUpdateSql(string tblName, SQLJoin &data, SQLJoin &where) {
    stringstream ss;
    ss << "update " << tblName << " set "
       << data.pairs()
       << " where " << where.pairs("and")
       << ";";
    return ss.str().c_str();
}

// 插入或者更新
string CDBWrap::GetUpdateOrInsertSql(string tblName, SQLJoin &data) {
    stringstream ss;
    ss << "insert into " << tblName << " set " << data.pairs()
       << " ON DUPLICATE KEY UPDATE " << data.pairs();
    return ss.str().c_str();
}

// 删除数据
string CDBWrap::GetDeleteSql(string tblName, SQLJoin &where) {
    stringstream ss;
    ss << "delete from " << tblName << " where " << where.pairs("and") << ";";
    return ss.str().c_str();
}

// select
string CDBWrap::GetSelectSql(string tblName, SQLJoin &fileds, SQLJoin &where) {
    stringstream ss;
    ss << "select " << fileds.keys() << " from " << tblName << " where " << where.pairs("and") << ";";

    return ss.str().c_str();
}

void CDBWrap::DebugSqlLog(const string& sql)
{
    if(m_debugLog){
        LOG_DEBUG("Exec Sql:{}",sql);
    }
}















