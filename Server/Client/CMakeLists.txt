cmake_minimum_required(VERSION 3.1.3)
PROJECT(Client)


#第三方库包含

include_directories(../Libs/include/rapidjson)
include_directories(../Libs/include/protobuf)
include_directories(../Libs/include/asio)
include_directories(../Libs/include/lua5.3)
include_directories(../Libs/include/spdlog)
#公共组件

include_directories(../Common)
include_directories(../Common/Pool)
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
aux_source_directory(../Common/Pool SUPER_SRC)
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

include_directories(./Manager)
aux_source_directory(./Manager SUPER_SRC)

aux_source_directory(./ SUPER_SRC)
LINK_DIRECTORIES(../Libs/lib)

IF(APPLE)
    SET (CMAKE_C_COMPILER "Library/Developer/CommandLineTools/usr/bin/clang++")
    SET (CMAKE_C_FLAGS "-Wall -std=c++11")
ELSEIF(UNIX)
    SET(CMAKE_CXX_COMPILER "ccache")
    SET(CMAKE_CXX_COMPILER_ARG1 "g++")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -std=gnu++11" )
ENDIF()
SET(CMAKE_BUILD_TYPE Debug)

ADD_EXECUTABLE(Client ${SUPER_SRC})
TARGET_LINK_LIBRARIES(Client Lua53 protobuf pthread dl ${MY_ADDLIB})

SET(EXECUTABLE_OUTPUT_PATH ../bin/)



