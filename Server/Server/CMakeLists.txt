cmake_minimum_required(VERSION 3.1.3)
PROJECT(Server)


#第三方库包含

include_directories(../Libs/include/rapidjson)
include_directories(../Libs/include/protobuf)
include_directories(../Libs/include/asio)
include_directories(../Libs/include/lua5.3)
include_directories(../Libs/include/mysql)
include_directories(../Libs/include/spdlog)
include_directories(../Libs/include/redis)
#公共组件
include_directories(../Common)
include_directories(../DataBase)
include_directories(../CoreLogic)
include_directories(../ServerData)

aux_source_directory(./main.cpp SUPER_SRC)

LINK_DIRECTORIES(../Libs/lib)
SET(CMAKE_CXX_COMPILER "ccache")
SET(CMAKE_CXX_COMPILER_ARG1 "g++")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -std=gnu++11" )
SET(CMAKE_BUILD_TYPE Debug)

ADD_EXECUTABLE(Server ${SUPER_SRC})
TARGET_LINK_LIBRARIES(Server Lua53 protobuf pthread hiredis mysqlclient yassl dl ${MY_ADDLIB})

SET(EXECUTABLE_OUTPUT_PATH ../bin/)



