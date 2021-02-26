
#pragma once

#include "time/time.hpp"
#include "utility/singleton.h"
#include "third/timer/timer_wheel.h"
#include "math/random.hpp"
#include "sol/sol.hpp"
#include <memory>
#include <vector>
#include "asio.hpp"
#include "lua_service/lua_service.h"
#include "network/tcp_server.h"
#include <string>

using namespace std;

extern int FrameworkMain(int argc, char* argv[]);

extern int ExitServer();

class CApplication : public AutoDeleteSingleton<CApplication> {
public:
    CApplication();

    ~CApplication();

public:
    bool PreInit();

    bool OverPreInit();

    void OverShutDown();

    uint64_t PreTick();

    void SetServerID(unsigned int svrid);

    uint32_t GetServerID();

    string GetUUID();

    //状态
    void SetStatus(uint8_t status);

    uint8_t GetStatus();

    //定时器
    void schedule(TimerEventInterface* event, uint64_t delta);

    void schedule_in_range(TimerEventInterface* event, uint64_t start, uint64_t end);

    //获得sol模块
    sol::state& GetSolLuaState();

    asio::io_context& GetAsioContext();

    //获取lua_service
    svrlib::lua_service* GetLuaService();

private:


public:
//具体实例去实现
    bool Initialize();

    void ShutDown();

    void ConfigurationChanged();

    void Tick(uint64_t diffTime);

    void ExceptionHandle();

private:
    string   m_uuid;                  // 进程唯一标识
    uint32_t m_uiServerID;
    uint8_t m_status;                 // 服务器状态
    uint64_t m_lastTick;              // 上次tick时间
    sol::state m_solLua;              // sol lua模块
    asio::io_context m_ioContext;
    svrlib::lua_service* m_luaService;         // lua server
    std::vector<std::shared_ptr<Network::TCPServer>> m_tcpServers;

    uint64_t m_wheelTime;             // wheel时间
    uint32_t m_wheelPrecision;        // wheel精度
    TimerWheel m_timers;              // wheel定时器
};


