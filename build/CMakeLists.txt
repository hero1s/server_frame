# cmake build list
# @author: toney
# serverframe cmake
cmake_minimum_required(VERSION 3.1)
project(svrlib VERSION 1.0 LANGUAGES C CXX)

include(./cmake_config/config_lib.cmake)
include(./cmake_config/macros.cmake)
include(./cmake_config/common_path.cmake)

clear_all_list()

add_include_path(../include)
add_include_path(${COMMON_INCLUDE})
add_include_path(${MYSQL_INCLUDE})
add_include_path(../include/third/lua)
add_include_path(../include/third/zlib)
add_include_path(../include/redis)
add_include_path(../include/modern)
add_include_path(../include/network)
add_include_path(../include/third)

add_source_files(../src/*.*)

build_static_lib(svrlib ../lib)
