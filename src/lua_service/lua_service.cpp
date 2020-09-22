//
// Created by toney on 2018/12/15.
//

#include "lua_service/lua_service.h"
#include "svrlib.h"

namespace svrlib {

    lua_service::lua_service(sol::state* p)
            :error_(false), lua_(p)
    {
    }

    lua_service::~lua_service()
    {
    }

    void lua_service::set_start(sol_function_t f)
    {
        LOG_DEBUG("set lua service start function");
        start_ = f;
    }

    void lua_service::set_dispatch(sol_function_t f)
    {
        LOG_DEBUG("set lua service dispatch function");
        dispatch_ = f;
    }

    void lua_service::set_exit(sol_function_t f)
    {
        LOG_DEBUG("set lua service exit function");
        exit_ = f;
    }

    void lua_service::start()
    {
        if (error_) return;
        try {
            if (start_.valid()) {
                auto result = start_();
                if (!result.valid()) {
                    sol::error err = result;
                    LOG_ERROR("lua start error {}", err.what());
                }
            }else{
                LOG_ERROR("start function is error");
            }
        }
        catch (std::exception& e) {
            LOG_ERROR("lua_service::start :{}", e.what());
            error();
        }
    }

    void lua_service::dispatch(uint16_t cmd, std::string msg)
    {
        if (error_) return;
        try {
            auto result = dispatch_(cmd, msg);
            if (!result.valid()) {
                sol::error err = result;
                LOG_ERROR("dispatch msg error {}", err.what());
            }else{
                LOG_ERROR("dispatch function is error");
            }
        }
        catch (std::exception& e) {
            LOG_ERROR("lua_service::dispatch:{}", e.what());
            error();
        }
    }

    void lua_service::exit()
    {
        if (!error_) {
            try {
                if (exit_.valid()) {
                    auto result = exit_();
                    if (!result.valid()) {
                        sol::error err = result;
                        LOG_ERROR("lua exit error {}", err.what());
                    }
                    return;
                }else{
                    LOG_ERROR("exit function is error");
                }
            }
            catch (std::exception& e) {
                LOG_ERROR("lua_service::exit :{}", e.what());
                error();
            }
        }
    }

    void lua_service::error()
    {
        error_ = true;
        std::string backtrace = lua_traceback(lua_->lua_state());
        LOG_ERROR("lua error {}", backtrace);
    }

    const char* lua_service::lua_traceback(lua_State* L)
    {
        luaL_traceback(L, L, NULL, 1);
        auto s = lua_tostring(L, -1);
        if (nullptr!=s) {
            return "";
        }
        return s;
    }

};



