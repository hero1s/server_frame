
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

//���ݿ������¼�
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

    vector<CBindParam> bindParams; // �󶨲���
    string sqlStr; // sql���
    string dbName; // ���ݿ���
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

    vector<CMysqlResultRow> vecData; // ����
    int64_t affectRow; // Ӱ������
};

class CDBTask : public CDBWrap {
public:
    explicit CDBTask(asio::io_context& context);

    virtual ~CDBTask();

    bool Init(const stDBConf& conf);

    void Start();

    //�ȴ��߳����
    void Join();

    //ֹͣ
    void Stop();

    void PushAndSelectDB(string dbName, const string& sql);

    // �첽��������
    void AsyncInsert(string dbName, string tblName, SQLJoin& data);

    // �첽��������
    void AsyncUpdate(string dbName, string tblName, SQLJoin& data, SQLJoin& where);

    // �첽������߸���
    void AsyncUpdateOrInsert(string dbName, string tblName, SQLJoin& data);

    // �첽ɾ������
    void AsyncDelete(string dbName, string tblName, SQLJoin& where);

    void SetDBName(string dbName);

    // �첽����
    void AsyncQuery(shared_ptr<CDBEventReq> pReq);

    shared_ptr<CDBEventReq> MallocDBEventReq();

private:
    // ����������ݿ����
    void AddDBReqEvent(shared_ptr<CDBEventReq> pReqEvent);

    // ������ݿⷵ���¼�
    void AddDBRepEvent(shared_ptr<CDBEventRep> pRepEvent, std::function<void(shared_ptr<CDBEventRep>& pRep)> callBack);

    // ���������¼�
    bool OnProcessEvent(shared_ptr<CDBEventReq> pReqEvent);

private:
    bool ConnectDB();

    void Run();

    // ͳ��ִ������
    void StatExeCount();

private:
    bool m_bRun;
    stDBConf m_conf;

    thread m_thread; // �̶߳���
    SyncQueue<shared_ptr<CDBEventReq>> m_QueueReq; //�������

    int64_t m_lastCountTime;
    int32_t m_Counts; // ��ǰ
    int32_t m_maxCounts; // ��ֵ
    string m_dbName;
    asio::io_context& m_context;
};
