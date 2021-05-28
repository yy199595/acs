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
include_directories(../Common/Core)
include_directories(../Common/Coroutine)
include_directories(../Common/Define)
include_directories(../Common/Global)
include_directories(../Common/Manager)
include_directories(../Common/Module)
include_directories(../Common/NetWork)
include_directories(../Common/Object)
include_directories(../Common/Other)
include_directories(../Common/Thread)
include_directories(../Common/Timer)
include_directories(../Common/Util)
include_directories(../Common/Script)
include_directories(../Common/Service)

aux_source_directory(../Common/Core SUPER_SRC)
aux_source_directory(../Common/Coroutine SUPER_SRC)
aux_source_directory(../Common/Define SUPER_SRC)
aux_source_directory(../Common/Global SUPER_SRC)
aux_source_directory(../Common/Manager SUPER_SRC)
aux_source_directory(../Common/Module SUPER_SRC)
aux_source_directory(../Common/NetWork SUPER_SRC)
aux_source_directory(../Common/Object SUPER_SRC)
aux_source_directory(../Common/Other SUPER_SRC)
aux_source_directory(../Common/Thread SUPER_SRC)
aux_source_directory(../Common/Timer SUPER_SRC)
aux_source_directory(../Common/Util SUPER_SRC)
aux_source_directory(../Common/Script SUPER_SRC)
aux_source_directory(../Common/Service SUPER_SRC)

include_directories(../ServerData)
include_directories(../ServerData/XCode)
include_directories(../ServerData/Protocol)

aux_source_directory(../ServerData/XCode SUPER_SRC)
aux_source_directory(../ServerData/Protocol SUPER_SRC)

include_directories(../CoreLogic)
include_directories(../CoreLogic/Manager)

aux_source_directory(../CoreLogic/Manager SUPER_SRC)

include_directories(../DataBase)
include_directories(../DataBase/Manager)
include_directories(../DataBase/MysqlClient)
include_directories(../DataBase/QueryResult)
include_directories(../DataBase/RedisClient)
include_directories(../DataBase/Script)

aux_source_directory(../DataBase/Manager SUPER_SRC)
aux_source_directory(../DataBase/MysqlClient SUPER_SRC)
aux_source_directory(../DataBase/QueryResult SUPER_SRC)
aux_source_directory(../DataBase/RedisClient SUPER_SRC)
aux_source_directory(../DataBase/Script SUPER_SRC)

aux_source_directory(./ SUPER_SRC)
LINK_DIRECTORIES(../Libs/lib)
SET(CMAKE_CXX_COMPILER "ccache")
SET(CMAKE_CXX_COMPILER_ARG1 "g++")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -std=gnu++11" )
SET(CMAKE_BUILD_TYPE Debug)

ADD_EXECUTABLE(Server ${SUPER_SRC})
TARGET_LINK_LIBRARIES(Server Lua53 protobuf pthread hiredis mysqlclient yassl dl ${MY_ADDLIB})

SET(EXECUTABLE_OUTPUT_PATH ../bin/)



