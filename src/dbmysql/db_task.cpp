
#include <dbmysql/db_task.h>
#include "svrlib.h"
#include "string/stringutility.h"
#include "time/time.hpp"

using namespace std;
using namespace svrlib;

CDBTask::CDBTask(asio::io_context &context)
        : m_QueueReq(100000),
          m_context(context) {
    m_bRun = false;
}

CDBTask::~CDBTask() {

}

bool CDBTask::Init(const stDBConf &conf) {
    m_Counts = 0;
    m_maxCounts = 0;
    m_lastCountTime = time::second();
    m_conf = conf;

    return ConnectDB();
}

void CDBTask::Start() {
    m_thread = thread(std::bind(&CDBTask::Run, this));
    time::sleep(500);
}

//等待线程完成
void CDBTask::Join() {
    m_thread.join();
}

void CDBTask::Stop() {
    m_bRun = false;
    m_QueueReq.Stop(); //让同步队列中的线程停止
}

void CDBTask::PushAndSelectDB(string dbName, const string &sql) {
    shared_ptr<CDBEventReq> pReq = MallocDBEventReq();
    pReq->sqlStr = sql;
    pReq->dbName = dbName;
    AddDBReqEvent(pReq);
}

// 异步插入数据
void CDBTask::AsyncInsert(string dbName, string tblName, SQLJoin &data) {
    PushAndSelectDB(dbName, GetInsertSql(tblName, data));
}

// 异步更新数据
void CDBTask::AsyncUpdate(string dbName, string tblName, SQLJoin &data, SQLJoin &where) {
    PushAndSelectDB(dbName, GetUpdateSql(tblName, data, where));
}

// 异步插入或者更新
void CDBTask::AsyncUpdateOrInsert(string dbName, string tblName, SQLJoin &data) {
    PushAndSelectDB(dbName, GetUpdateOrInsertSql(tblName, data));
}

// 异步删除数据
void CDBTask::AsyncDelete(string dbName, string tblName, SQLJoin &where) {
    PushAndSelectDB(dbName, GetDeleteSql(tblName, where));
}

void CDBTask::SetDBName(string dbName) {
    m_dbName = dbName;
}

bool CDBTask::ConnectDB() {
    LOG_DEBUG("DBTask connect mysql:{},{},{},{},{}", m_conf.sHost, m_conf.sUser, m_conf.sPwd, m_conf.sDBName, m_conf.uPort);
    return dbOpen(m_conf.sHost.c_str(), m_conf.sUser.c_str(), m_conf.sPwd.c_str(), m_conf.sDBName.c_str(), m_conf.uPort);
}

void CDBTask::Run() {
    m_bRun = true;
    std::deque<shared_ptr<CDBEventReq>> list;
    while (m_bRun)
    {
        m_QueueReq.Take(list);
        while (!list.empty())
        {
            if (!m_bRun)
            {
                return;
            }
            auto pReq = list.front();
            list.pop_front();
            // 关闭后查询类sql不再处理
            if (m_bRun)
            {
                if (OnProcessEvent(pReq) == false)
                {
                    if (!m_clDatabase.isOpen())
                    {
                        ConnectDB();
                        continue;
                    }
                    else
                    {
                        LOG_ERROR("sql exec fail:{}", pReq->sqlStr);
                    }
                }
            }
            StatExeCount();
        }
    }
    LOG_DEBUG("CAsyncTask run over:{}", m_dbName);
}

// 统计执行数量
void CDBTask::StatExeCount() {
    if ((time::getSysTime() - m_lastCountTime) > 300)
    {
        int32_t minCount = m_Counts / 5;
        m_maxCounts = std::max(m_maxCounts, minCount);
        LOG_DEBUG("********DBTask thread {} presss {} count*******max:{}", m_dbName, minCount, m_maxCounts);
        m_Counts = 0;
        m_lastCountTime = time::getSysTime();
    }
}

// 异步处理
void CDBTask::AsyncQuery(shared_ptr<CDBEventReq> pReq) {
    AddDBReqEvent(pReq);
}

shared_ptr<CDBEventReq> CDBTask::MallocDBEventReq() {
    shared_ptr<CDBEventReq> pReq(std::make_shared<CDBEventReq>());
    return pReq;
}

// 添加请求数据库操作
void CDBTask::AddDBReqEvent(shared_ptr<CDBEventReq> pReqEvent) {
    m_QueueReq.Put(pReqEvent);
}

// 添加数据库返回事件
void CDBTask::AddDBRepEvent(shared_ptr<CDBEventRep> pRepEvent, std::function<void(shared_ptr<CDBEventRep> &pRep)> callBack) {
    m_context.post(std::bind(callBack, pRepEvent));
}

// 处理请求事件
bool CDBTask::OnProcessEvent(shared_ptr<CDBEventReq> pReqEvent) {
    m_Counts++;
    if (pReqEvent->dbName.length() > 2)
    {
        if (!dbSelect(pReqEvent->dbName.c_str()))
        {
            return false;
        }
    }
    if (pReqEvent->callBack == nullptr)
    {
        if (pReqEvent->bindParams.empty())
        {
            return ExeSql(pReqEvent->sqlStr);
        }
        else
        {
            return ExeBindSql(pReqEvent->sqlStr, pReqEvent->bindParams);
        }
    }

    shared_ptr<CDBEventRep> pRep(std::make_shared<CDBEventRep>());

    if (Query(pReqEvent->sqlStr.c_str(), pRep->vecData, pRep->affectRow) == 0)
    {
        AddDBRepEvent(pRep, pReqEvent->callBack);
        return true;
    }

    return false;
}



