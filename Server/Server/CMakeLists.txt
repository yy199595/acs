cmake_minimum_required(VERSION 3.1.3)
PROJECT(Server)
add_definitions(-D _DEBUG)

#第三方库包含

SET(LinkDir ${PROJECT_SOURCE_DIR}../Libs/lib)
SET(LibDir ${PROJECT_SOURCE_DIR}../Libs)
SET(CommonDir ${PROJECT_SOURCE_DIR}../Common)
SET(DataBaseDir ${PROJECT_SOURCE_DIR}../DataBase)
SET(CoreLogicDir ${PROJECT_SOURCE_DIR}../CoreLogic)
SET(ServerDataDir ${PROJECT_SOURCE_DIR}../ServerData)
SET(OutputDir ${PROJECT_SOURCE_DIR}../bin)

message(${LibDir})
message(${LinkDir})
message(${CommonDir})
message(${DataBaseDir})
message(${CoreLogicDir})
message(${ServerDataDir})
message(${OutputDir})


include_directories(${LibDir}/include/rapidjson)
include_directories(${LibDir}include/protobuf)
include_directories(${LibDir}include/asio)
include_directories(${LibDir}include/lua5.3)
include_directories(${LibDir}include/mysql)
include_directories(${LibDir}include/spdlog)
include_directories(${LibDir}include/redis)
#公共组件

include_directories(${CommonDir})
include_directories(${CommonDir}/Core)
include_directories(${CommonDir}/Coroutine)
include_directories(${CommonDir}/Define)
include_directories(${CommonDir}/Global)
include_directories(${CommonDir}/Manager)
include_directories(${CommonDir}/Module)
include_directories(${CommonDir}/NetWork)
include_directories(${CommonDir}/Object)
include_directories(${CommonDir}/Other)
include_directories(${CommonDir}/Thread)
include_directories(${CommonDir}/Timer)
include_directories(${CommonDir}/Util)
include_directories(${CommonDir}/Script)
include_directories(${CommonDir}/Service)
include_directories(${CommonDir}/Pool)


aux_source_directory(${CommonDir}/Core SUPER_SRC)
aux_source_directory(${CommonDir}/Coroutine SUPER_SRC)
aux_source_directory(${CommonDir}/Define SUPER_SRC)
aux_source_directory(${CommonDir}/Global SUPER_SRC)
aux_source_directory(${CommonDir}/Manager SUPER_SRC)
aux_source_directory(${CommonDir}/Module SUPER_SRC)
aux_source_directory(${CommonDir}/NetWork SUPER_SRC)
aux_source_directory(${CommonDir}/Object SUPER_SRC)
aux_source_directory(${CommonDir}/Other SUPER_SRC)
aux_source_directory(${CommonDir}/Thread SUPER_SRC)
aux_source_directory(${CommonDir}/Timer SUPER_SRC)
aux_source_directory(${CommonDir}/Util SUPER_SRC)
aux_source_directory(${CommonDir}/Script SUPER_SRC)
aux_source_directory(${CommonDir}/Service SUPER_SRC)
aux_source_directory(${CommonDir}/Pool SUPER_SRC)

include_directories(${ServerDataDir})
include_directories(${ServerDataDir}/XCode)
include_directories(${ServerDataDir}/Protocol)

aux_source_directory(${ServerDataDir}/XCode SUPER_SRC)
aux_source_directory(${ServerDataDir}/Protocol SUPER_SRC)

include_directories(${CoreLogicDir})
include_directories(${CoreLogicDir}/Manager)

aux_source_directory(${CoreLogicDir}/Manager SUPER_SRC)

include_directories(${DataBaseDir})
include_directories(${DataBaseDir}/Service)
include_directories(${DataBaseDir}/Manager)
include_directories(${DataBaseDir}/MysqlClient)
include_directories(${DataBaseDir}/QueryResult)
include_directories(${DataBaseDir}/RedisClient)

aux_source_directory(${DataBaseDir}/Service SUPER_SRC)
aux_source_directory(${DataBaseDir}/Manager SUPER_SRC)
aux_source_directory(${DataBaseDir}/MysqlClient SUPER_SRC)
aux_source_directory(${DataBaseDir}/QueryResult SUPER_SRC)
aux_source_directory(${DataBaseDir}/RedisClient SUPER_SRC)
aux_source_directory(${DataBaseDir}/Script SUPER_SRC)

aux_source_directory(${PROJECT_SOURCE_DIR} SUPER_SRC)

LINK_DIRECTORIES(${LinkDir})

IF(APPLE)
    SET (CMAKE_C_COMPILER "Library/Developer/CommandLineTools/usr/bin/clang++")
    SET (CMAKE_C_FLAGS "-Wall -std=c++11")
ELSEIF(UNIX)
    SET(CMAKE_CXX_COMPILER "ccache")
    SET(CMAKE_CXX_COMPILER_ARG1 "g++")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -std=gnu++11" )
ENDIF()

SET(EX_PLATFORM 64)
SET(EX_PLATFORM_NAME "x64")

SET(CMAKE_BUILD_TYPE Debug)

IF(UNIX)
TARGET_LINK_LIBRARIES(Server Lua53 protobuf pthread hiredis mysqlclient yassl dl ${MY_ADDLIB})
ELSEIF(WIN32)
TARGET_LINK_LIBRARIES(Server Lua53 libprotobufd mysqlclient hiredis ${MY_ADDLIB})
ENDIF()

SET(EXECUTABLE_OUTPUT_PATH ${OutputDir})



