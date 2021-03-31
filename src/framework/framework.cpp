
#include "framework.h"
#include "asio.hpp"
#include "cmdline.h"
#include "file/filehelper.h"
#include "framework/application.h"
#include "helper/helper.h"
#include "lua_service/lua_bind.h"
#include "spdlog/spdlog.h"
#include "time/time.hpp"
#include "utility/comm_macro.h"
#include "utility/profile_manager.h"
#include <chrono>
#include <ctime>
#include <fcntl.h>
#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace svrlib;
using std::vector;

static uint32_t TICK_MAX_INTERVAL = 100;
static uint32_t TICK_MIN_INTERVAL = 10;

string CFrameWork::m_confFilename = "";

CFrameWork::CFrameWork()
{
    m_sleepTime = TICK_MIN_INTERVAL;
}

CFrameWork::~CFrameWork()
{
}

void CFrameWork::Run()
{

    m_pTimer = make_shared<asio::system_timer>(CApplication::Instance().GetAsioContext());
    m_pTimer->expires_from_now(std::chrono::milliseconds(m_sleepTime));
    m_pTimer->async_wait(std::bind(&CFrameWork::TimerTick, this, std::placeholders::_1));

    try {
        CApplication::Instance().GetAsioContext().run();
    } catch (std::exception& e) {
        std::cout << "asio error " << e.what() << std::endl;
        LOG_ERROR("asio error:{}", e.what());
    }
}

void CFrameWork::ShutDown()
{
    LOG_DEBUG("FrameWork ShutDown:{}", CApplication::Instance().GetServerID());
    PROFILE_SHUTDOWN("profile", CApplication::Instance().GetServerID());
    CApplication::Instance().ShutDown();
    CApplication::Instance().OverShutDown();

    m_pTimer->cancel();
    m_pTimer->expires_from_now(std::chrono::seconds(2));
    m_pTimer->async_wait(std::bind([](const std::error_code& err) {
        CApplication::Instance().GetAsioContext().stop();
    },
        std::placeholders::_1));
}

void CFrameWork::TimerTick(const std::error_code& err)
{
    if (!err) {
        auto startTime = time::millisecond();
        CApplication::Instance().Tick(CApplication::Instance().PreTick());
        auto costTime = time::millisecond() - startTime;
        if (costTime > TICK_MAX_INTERVAL) {
            LOG_ERROR("************** preccess tick time out:{} *********", costTime);
            m_sleepTime = TICK_MIN_INTERVAL;
        } else {
            m_sleepTime = TICK_MAX_INTERVAL - costTime;
        }

        m_pTimer->expires_from_now(std::chrono::milliseconds(m_sleepTime));
        m_pTimer->async_wait(std::bind(&CFrameWork::TimerTick, this, std::placeholders::_1));
    } else {
        LOG_ERROR("asio timer is error or cancel,shutdown");
        CApplication::Instance().GetAsioContext().stop();
    }
}

void CFrameWork::InitializeEnvironment(int argc, char* argv[])
{
    PROFILE_INIT();

    ParseInputParam(argc, argv);

    signal(SIGUSR2, ReloadConfig);
    signal(SIGUSR1, StopRun);

    LoadConfig();
    //提前写入进程id,防止多次启动
    WritePidToFile();

    CApplication::Instance().PreInit();
    InitSpdlog();

    CApplication::Instance().OverPreInit();
    bool bRet = CApplication::Instance().Initialize();
    if (bRet == false) {
        exit(1);
    } else {
        LOG_DEBUG("start server success....");
        WritePidToFile();
    }
}

void CFrameWork::InitSpdlog()
{
    if (m_serverCfg.logasync > 0) { //异步模式
        auto log = spdlog::rotating_logger_st("log", m_serverCfg.logname, m_serverCfg.logsize, m_serverCfg.logdays);
        log->flush_on(spdlog::level::err);
        log->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%n][%l][tid %t] %v");
        log->set_level(spdlog::level::level_enum(m_serverCfg.loglv));
        spdlog::set_async_mode(m_serverCfg.logasync);

    } else {
        auto log = spdlog::rotating_logger_mt("log", m_serverCfg.logname, m_serverCfg.logsize, m_serverCfg.logdays);
        log->flush_on(spdlog::level::trace);
        log->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%n][%l][tid %t] %v");
        log->set_level(spdlog::level::level_enum(m_serverCfg.loglv));
        spdlog::set_sync_mode();
    }
}

void CFrameWork::ParseInputParam(int argc, char* argv[])
{
    cmdline::parser a;
    a.add<int>("sid", '\0', "server id", false, 1, cmdline::range(1, 100000));
    a.add<string>("cfg", '\0', "cfg file name", false, "");
    a.add<int>("loglv", '\0', "log level", false, 0, cmdline::range(0, 7));
    a.add<int>("logsize", '\0', "log size", false, 52428800);
    a.add<int>("logdays", '\0', "log save days", false, 5, cmdline::range(1, 30));
    a.add<int>("logasync", '\0', "log async size", false, 0);
    a.add<string>("logname", '\0', "log name", false, "log.txt");

    bool ok = a.parse(argc, argv);
    if (!ok) {
        printf("%s : %s \n\r", a.error().c_str(), a.usage().c_str());
        exit(1);
        return;
    }
    CApplication::Instance().SetServerID(a.get<int>("sid"));
    string cfgName = a.get<string>("cfg");
    m_serverCfg.loglv = a.get<int>("loglv");
    m_serverCfg.logsize = a.get<int>("logsize");
    m_serverCfg.logdays = a.get<int>("logdays");
    m_serverCfg.logasync = a.get<int>("logasync");
    m_serverCfg.logname = CStringUtility::FormatToString("%d_%s", CApplication::Instance().GetServerID(),
        a.get<string>("logname").c_str());
    m_confFilename = cfgName;
}

void CFrameWork::LoadConfig()
{
    CApplication::Instance().GetSolLuaState().do_file(m_confFilename);
}

void CFrameWork::ReloadConfig(int iSig)
{
    if (iSig == SIGUSR2) {
        LOG_DEBUG("program reload config...");
        CApplication::Instance().GetAsioContext().post([]() {
            CFrameWork::Instance().LoadConfig();
            CApplication::Instance().ConfigurationChanged();
        });
    }
}

void CFrameWork::StopRun(int iSig)
{
    if (iSig == SIGUSR1) {
        LOG_DEBUG("signal program exiting...");
        CFrameWork::Instance().ShutDown();
    }
}

void CFrameWork::WritePidToFile()
{
    std::ostringstream oss;
    oss << getpid();
    std::string strShFileName = CStringUtility::FormatToString("pid_%d.txt", CApplication::Instance().GetServerID());
    CFileHelper oFile(strShFileName.c_str(), CFileHelper::MOD_WRONLY_TRUNC);
    oFile.Write(0, oss.str().c_str(), oss.str().length());
    oFile.Close();
    time::sleep(10);
    oss.str("");
    oss << "chmod 777 " << strShFileName;

    ::system(oss.str().c_str());
}
