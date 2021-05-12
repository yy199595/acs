cmake_minimum_required(VERSION 3.1.3)
PROJECT(Server)

AUX_SOURCE_DIRECTORY(./ SCENE_SRC)

INCLUDE_DIRECTORIES(./)
INCLUDE_DIRECTORIES(../Libs/include/rapidjson)
INCLUDE_DIRECTORIES(../Libs/include/protobuf)
INCLUDE_DIRECTORIES(../Libs/include/asio)
INCLUDE_DIRECTORIES(../Libs/include/lua5.3)
INCLUDE_DIRECTORIES(../Libs/include/mysql)
INCLUDE_DIRECTORIES(../Libs/include/spdlog)
INCLUDE_DIRECTORIES(../Libs/include/redis)

INCLUDE_DIRECTORIES(../Common)
INCLUDE_DIRECTORIES(../DataBase)
INCLUDE_DIRECTORIES(../CoreLogic)
INCLUDE_DIRECTORIES(../ServerData)

LINK_DIRECTORIES(../../Libs/lib)
SET(CMAKE_CXX_COMPILER "ccache")
SET(CMAKE_CXX_COMPILER_ARG1 "g++") 

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -std=gnu++11" )
SET(CMAKE_BUILD_TYPE Debug)

ADD_EXECUTABLE(Server ${SCENE_SRC})
TARGET_LINK_LIBRARIES(Server Common protobuf Lua53 pthread hiredis mysqlclient yassl DataBase CoreLogic ServerData dl  ${MY_ADDLIB})

SET(EXECUTABLE_OUTPUT_PATH ../../bin/)



