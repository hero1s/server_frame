#pragma once

#include <string>
#include "sol/sol.hpp"
#include "utility/noncopyable.hpp"

namespace svrlib {

    class lua_service;

    class lua_bind : public svrlib::noncopyable {
    public:
        explicit lua_bind(sol::state &_lua);

        ~lua_bind();

        void export_lua_bind();

        void registerlib(const char *name, lua_CFunction f);

        void registerlib(const char *name, const sol::table &);

        void add_lua_cpath(std::vector<std::string> cpaths);

        void add_lua_path(std::vector<std::string> paths);

        void add_lua_dir_path(std::string dirPath);

        void reload_lua_file(std::string fileName);

        void reload_lua_dir(std::string dirPath);

    protected:
        void bind_conf();

        void bind_util();

        void bind_log();

        void bind_service();

    private:
        sol::state &lua;
    };

};


