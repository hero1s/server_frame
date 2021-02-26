
#include <iostream>
#include "framework.h"
#include "framework/application.h"
#include <malloc.h>
#include "sol/sol.hpp"
#include "helper/helper.h"
#include "lua/lua.h"
#include "spdlog/spdlog.h"
#include "string/stringutility.h"
#include "utility/comm_macro.h"
#include "config/config.h"
#include "lua_service/lua_bind.h"
#include "ebus/frame_event.hpp"
#include <memory>
#include "time/time.hpp"
#include "utility/puid.hpp"

using namespace svrlib;

int FrameworkMain(int argc, char *argv[]) {
    if (argc <= 0 || argv == NULL) {
        throw "The input argument for FrameworkMain is illegal!";
    }
    //mallopt(M_ARENA_MAX, 1);

    try {
        CFrameWork::Instance().InitializeEnvironment(argc, argv);
        CFrameWork::Instance().Run();
    }
    catch (char const *what) {
        std::cout << what << std::endl;
        LOG_ERROR("process exit {}", what);
        CApplication::Instance().ShutDown();
    }
    catch (sol::error &e) {
        std::cout << "sol error " << e.what() << std::endl;
        LOG_ERROR("sol error:{}", e.what());
        CApplication::Instance().ShutDown();
    }
    catch (std::system_error &e){
        std::cout << "std::system_error:" << e.what() << std::endl;
        LOG_ERROR("std::system_error:{}",e.what());
        CApplication::Instance().ShutDown();
    }
    spdlog::drop_all();
    return 0;
}

int ExitServer()
{
    LOG_DEBUG("call program exiting...");
    CFrameWork::Instance().ShutDown();
    return 0;
}

CApplication::CApplication() {
    m_status = 0;
    m_lastTick = 0;
    m_wheelTime = 0;
    m_wheelPrecision = 10;
    m_solLua.open_libraries();
    m_luaService = new svrlib::lua_service(&m_solLua);
    m_uuid = svrlib::uuid::generate();
}

CApplication::~CApplication() {
    m_status = 0;
    SAFE_DELETE(m_luaService);
}

bool CApplication::PreInit() {
    lua_bind bind(m_solLua);
    bind.export_lua_bind();
    
    return true;
}

bool CApplication::OverPreInit(){

    return true;
}

void CApplication::OverShutDown() {
    for(auto s:m_tcpServers){
        s->Stop();
    }
    //m_tcpServers.clear();
}

uint64_t CApplication::PreTick() {
    // 驱动时钟
    time::getSystemTick64(true);// 更新tick
    time::getSysTime(true);
    std::srand(time::getSysTime());
    if (m_lastTick == 0) {
        m_lastTick = time::getSystemTick64();
    }
    uint64_t curTime = time::getSystemTick64();
    int64_t delta = curTime - m_lastTick;
    if (delta > 0) {
        m_wheelTime += delta;
        m_timers.advance(m_wheelTime/m_wheelPrecision);
        m_wheelTime = m_wheelTime%m_wheelPrecision;
        m_lastTick = curTime;
    }

    return delta;
}

void CApplication::SetServerID(unsigned int svrid) {
    m_uiServerID = svrid;
}

uint32_t CApplication::GetServerID() {
    return m_uiServerID;
}

string CApplication::GetUUID(){
    return m_uuid;
}

//状态
void CApplication::SetStatus(uint8_t status) {
    m_status = status;
}

uint8_t CApplication::GetStatus() {
    return m_status;
}

void CApplication::schedule(TimerEventInterface *event, uint64_t delta) {
    m_timers.schedule(event, delta/m_wheelPrecision);
}

void CApplication::schedule_in_range(TimerEventInterface *event, uint64_t start, uint64_t end) {
    m_timers.schedule_in_range(event, start/m_wheelPrecision, end/m_wheelPrecision);
}

//获得sol模块
sol::state &CApplication::GetSolLuaState() {
    return m_solLua;
}

asio::io_context &CApplication::GetAsioContext() {
    return m_ioContext;
}
//获取lua_service
svrlib::lua_service* CApplication::GetLuaService(){
    return m_luaService;
}


