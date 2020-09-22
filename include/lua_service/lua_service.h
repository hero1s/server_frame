//
// Created by toney on 2018/12/15.
//
#pragma  once

#include "sol/sol.hpp"
#include <string>

namespace svrlib {

    class lua_service {
    public:
        using sol_function_t = sol::function;

        explicit lua_service(sol::state* p);

        ~lua_service();

        void set_start(sol_function_t f);

        void set_dispatch(sol_function_t f);

        void set_exit(sol_function_t f);

    public:
        void start();

        void exit();

        void dispatch(uint16_t cmd, std::string msg);

    protected:
        void error();

        const char* lua_traceback(lua_State* _state);

    private:
        bool error_;
        sol::state* lua_;
        sol_function_t start_;
        sol_function_t dispatch_;
        sol_function_t exit_;

    };

};