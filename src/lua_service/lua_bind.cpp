
#include "lua_service/lua_bind.h"
#include "config/config.h"
#include "file/directory.hpp"
#include "lua_service/lua_service.h"
#include "svrlib.h"
#include "time/time.hpp"

namespace fs = std::experimental::filesystem;

namespace svrlib {
static const std::string LUA_PATH_STR = "/?.lua;";
static const std::string LUA_CPATH_STR = "/?.so;";

lua_bind::lua_bind(sol::state& _lua)
    : lua(_lua)
{
}

lua_bind::~lua_bind()
{
}

void lua_bind::export_lua_bind()
{
    bind_conf();
    bind_log();
    bind_util();
    bind_service();
}

void lua_bind::registerlib(const char* name, lua_CFunction f)
{
    lua_State* L = lua.lua_state();
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload"); /* get 'package.preload' */
    lua_pushcfunction(L, f);
    lua_setfield(L, -2, name); /* package.preload[name] = f */
    lua_pop(L, 2); /* pop 'package' and 'preload' tables */
}

void lua_bind::registerlib(const char* name, const sol::table& module)
{
    lua_State* L = lua.lua_state();
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "loaded"); /* get 'package.preload' */
    module.push();
    lua_setfield(L, -2, name); /* package.preload[name] = f */
    lua_pop(L, 2); /* pop 'package' and 'preload' tables */
}

void lua_bind::add_lua_cpath(std::vector<std::string> cpaths)
{
    std::string strpath;
    strpath.append("package.cpath ='");
    for (auto& v : cpaths) {
        strpath.append(v.data(), v.size());
        strpath.append(LUA_CPATH_STR);
    }
    strpath.append("'..package.cpath");
    lua.script(strpath);
    LOG_DEBUG("add lua cpath:{}", strpath);
}

void lua_bind::add_lua_path(std::vector<std::string> paths)
{
    std::string strpath;
    strpath.append("package.path ='");
    for (auto& v : paths) {
        strpath.append(v.data(), v.size());
        strpath.append(LUA_PATH_STR);
    }
    strpath.append("'..package.path");
    lua.script(strpath);
    LOG_DEBUG("add lua path:{}", strpath);
}

void lua_bind::add_lua_dir_path(std::string dirPath)
{
    std::string curDir = fs::current_path();
    curDir.append("/");
    //LOG_DEBUG("reload cur dir:{}", curDir);
    directory::traverse_folder(dirPath, 10, [&](const fs::path& path, bool dir) {
        if (dir) {
            LOG_DEBUG("file path is:{}", path.c_str());
            std::string fullName = path;
            CStringUtility::DeleteSubStr(fullName, curDir);
            this->add_lua_path({ fullName });
        }
        return true;
    });
}

void lua_bind::reload_lua_file(std::string fileName)
{
    CStringUtility::DeleteSubStr(fileName, ".lua");
    CStringUtility::ReplaceStr(fileName, ".", "/");
    LOG_DEBUG("reload lua file:{}", fileName);
    std::string reloadStr = CStringUtility::FormatToString("package.loaded['%s'] = nil;require('%s');",
        fileName.c_str(), fileName.c_str());
    lua.script(reloadStr);
}

void lua_bind::reload_lua_dir(std::string dirPath)
{
    std::string curDir = fs::current_path();
    curDir.append("/");
    //LOG_DEBUG("reload cur dir:{}", curDir);
    directory::traverse_folder(dirPath, 10, [&](const fs::path& path, bool dir) {
        if (!dir) {
            if (path.extension() == ".lua") {
                //LOG_DEBUG("file path is:{}", path.c_str());
                std::string fullName = path;
                CStringUtility::DeleteSubStr(fullName, curDir);
                this->reload_lua_file(fullName);
            }
        }
        return true;
    });
}

void lua_bind::bind_conf()
{
    lua.new_usertype<stDBConf>(
        "stDBConf",
        "SetDBInfo", &stDBConf::SetDBInfo);
    lua.new_usertype<stRedisConf>(
        "stRedisConf",
        "SetRedisHost", &stRedisConf::SetRedisHost);
    lua.new_usertype<stServerCfg>(
        "stServerCfg",
        "SetLogInfo", &stServerCfg::SetLogInfo);
}

void lua_bind::bind_util()
{
    lua.set_function("getSysTime", []() {
        return time::getSysTime();
    });
    lua.set_function("getTick", []() {
        return time::millisecond();
    });
    lua.set_function("curTimeStr", []() {
        return time::time_format(time::getSysTime());
    });
}

void lua_bind::bind_log()
{
    lua.set_function("log_debug", [](const char* logStr) {
        LOG_DEBUG("{}", logStr);
    });
    lua.set_function("log_error", [](const char* logStr) {
        LOG_ERROR("{}", logStr);
    });
    lua.set_function("log_info", [](const char* logStr) {
        LOG_INFO("{}", logStr);
    });
}

void lua_bind::bind_service()
{
    lua.new_usertype<lua_service>(
        "lua_service",
        "set_start", &lua_service::set_start,
        "set_exit", &lua_service::set_exit,
        "set_dispatch", &lua_service::set_dispatch);
}
};
