
#pragma once

#include "asio.hpp"
#include "config/config.h"
#include "utility/singleton.h"
#include <memory>
#include <string>

class CFrameWork : public AutoDeleteSingleton<CFrameWork> {
public:
    CFrameWork();
    ~CFrameWork();

    void Run();
    void ShutDown();
    void TimerTick(const std::error_code& err);
    void InitializeEnvironment(int argc, char* argv[]);
    void InitSpdlog();

    static void ReloadConfig(int iSig);
    static void StopRun(int iSig);

private:
    void ParseInputParam(int argc, char* argv[]);
    void LoadConfig();
    void WritePidToFile();

    uint64_t m_sleepTime;
    std::shared_ptr<asio::system_timer> m_pTimer = nullptr;
    svrlib::stServerCfg m_serverCfg;
    static std::string m_confFilename;
};
