
#pragma once

#include "config/config.h"
#include "dbmysql/db_wrap.h"
#include "dbmysql/dbmysql.h"
#include "string/string_functions.h"
#include "svrlib.h"
#include "third/asio.hpp"
#include "thread/sync_queue.hpp"
#include "utility/basic_functions.h"
#include "utility/basic_types.h"
#include <chrono>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

using namespace std;
using namespace svrlib;

class CDBEventRep;

//数据库请求事件
class CDBEventReq {
public:
    CDBEventReq()
    {
        Reset();
    }

    virtual ~CDBEventReq()
    {
    }

    void Reset()
    {
        sqlStr = "";
        dbName = "";
        bindParams.clear();
        callBack = nullptr;
    }

    template <typename T>
    CDBEventReq& pushParam(const T& val)
    {
        CBindParam param;
        param.Bind(val);
        bindParams.push_back(param);
        return *this;
    }

    CDBEventReq& pushBlobParam(const void* pData, int iLen)
    {
        CBindParam param;
        param.BindBlob(pData, iLen);
        bindParams.push_back(param);
        return *this;
    }

    vector<CBindParam> bindParams; // 绑定参数
    string sqlStr; // sql语句
    string dbName; // 数据库名
    std::function<void(shared_ptr<CDBEventRep>& pRep)> callBack;
};

class CDBEventRep {
public:
    CDBEventRep()
    {
        Reset();
    }

    virtual ~CDBEventRep()
    {
    }

    void Reset()
    {
        vecData.clear();
        affectRow = 0;
    }

    vector<CMysqlResultRow> vecData; // 数据
    int64_t affectRow; // 影响行数
};

class CDBTask : public CDBWrap {
public:
    explicit CDBTask(asio::io_context& context);

    virtual ~CDBTask();

    bool Init(const stDBConf& conf);

    void Start();

    //等待线程完成
    void Join();

    //停止
    void Stop();

    void PushAndSelectDB(string dbName, const string& sql);

    // 异步插入数据
    void AsyncInsert(string dbName, string tblName, SQLJoin& data);

    // 异步更新数据
    void AsyncUpdate(string dbName, string tblName, SQLJoin& data, SQLJoin& where);

    // 异步插入或者更新
    void AsyncUpdateOrInsert(string dbName, string tblName, SQLJoin& data);

    // 异步删除数据
    void AsyncDelete(string dbName, string tblName, SQLJoin& where);

    void SetDBName(string dbName);

    // 异步处理
    void AsyncQuery(shared_ptr<CDBEventReq> pReq);

    shared_ptr<CDBEventReq> MallocDBEventReq();

private:
    // 添加请求数据库操作
    void AddDBReqEvent(shared_ptr<CDBEventReq> pReqEvent);

    // 添加数据库返回事件
    void AddDBRepEvent(shared_ptr<CDBEventRep> pRepEvent, std::function<void(shared_ptr<CDBEventRep>& pRep)> callBack);

    // 处理请求事件
    bool OnProcessEvent(shared_ptr<CDBEventReq> pReqEvent);

private:
    bool ConnectDB();

    void Run();

    // 统计执行数量
    void StatExeCount();

private:
    bool m_bRun;
    stDBConf m_conf;

    thread m_thread; // 线程对象
    SyncQueue<shared_ptr<CDBEventReq>> m_QueueReq; //请求队列

    int64_t m_lastCountTime;
    int32_t m_Counts; // 当前
    int32_t m_maxCounts; // 峰值
    string m_dbName;
    asio::io_context& m_context;
};
