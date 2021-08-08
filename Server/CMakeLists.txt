cmake_minimum_required(VERSION 3.1.3)


PROJECT(Server)

#第三方库包含

include_directories(${PROJECT_SOURCE_DIR}/Libs/include/rapidjson)
include_directories(${PROJECT_SOURCE_DIR}/Libs/include/protobuf)
include_directories(${PROJECT_SOURCE_DIR}/Libs/include/asio)
include_directories(${PROJECT_SOURCE_DIR}/Libs/include/lua5.3)
include_directories(${PROJECT_SOURCE_DIR}/Libs/include/mysql)
include_directories(${PROJECT_SOURCE_DIR}/Libs/include/spdlog)
include_directories(${PROJECT_SOURCE_DIR}/Libs/include/redis)
#公共组件

include_directories(${PROJECT_SOURCE_DIR}/Common)
include_directories(${PROJECT_SOURCE_DIR}/Common/Core)
include_directories(${PROJECT_SOURCE_DIR}/Common/Coroutine)
include_directories(${PROJECT_SOURCE_DIR}/Common/Define)
include_directories(${PROJECT_SOURCE_DIR}/Common//Global)
include_directories(${PROJECT_SOURCE_DIR}/Common/Manager/)
include_directories(${PROJECT_SOURCE_DIR}/Common/Module)
include_directories(${PROJECT_SOURCE_DIR}/Common/NetWork)
include_directories(${PROJECT_SOURCE_DIR}/Common/Object)
include_directories(${PROJECT_SOURCE_DIR}/Common/Other)
include_directories(${PROJECT_SOURCE_DIR}/Common/Thread)
include_directories(${PROJECT_SOURCE_DIR}/Common/Timer)
include_directories(${PROJECT_SOURCE_DIR}/Common/Util)
include_directories(${PROJECT_SOURCE_DIR}/Common/Script)
include_directories(${PROJECT_SOURCE_DIR}/Common/Service)
include_directories(${PROJECT_SOURCE_DIR}/Common/Pool)

#ux_source_directory(${PROJECT_SOURCE_DIR}/Common/Core CommonSource)
aux_source_directory(${PROJECT_SOURCE_DIR}/Common/Coroutine CommonSource)
aux_source_directory(${PROJECT_SOURCE_DIR}/Common/Define CommonSource)
aux_source_directory(${PROJECT_SOURCE_DIR}/Common/Global CommonSource)
aux_source_directory(${PROJECT_SOURCE_DIR}/Common/Manager CommonSource)
aux_source_directory(${PROJECT_SOURCE_DIR}/Common/Module CommonSource)
aux_source_directory(${PROJECT_SOURCE_DIR}/Common/NetWork CommonSource)
aux_source_directory(${PROJECT_SOURCE_DIR}/Common/Object CommonSource)
aux_source_directory(${PROJECT_SOURCE_DIR}/Common/Other CommonSource)
aux_source_directory(${PROJECT_SOURCE_DIR}/Common/Thread CommonSource)
aux_source_directory(${PROJECT_SOURCE_DIR}/Common/Timer CommonSource)
aux_source_directory(${PROJECT_SOURCE_DIR}/Common/Util CommonSource)
aux_source_directory(${PROJECT_SOURCE_DIR}/Common/Script CommonSource)
aux_source_directory(${PROJECT_SOURCE_DIR}/Common/Service CommonSource)
aux_source_directory(${PROJECT_SOURCE_DIR}/Common/Pool CommonSource)

include_directories(${PROJECT_SOURCE_DIR}/Protocol)
include_directories(${PROJECT_SOURCE_DIR}/Protocol/XCode)
include_directories(${PROJECT_SOURCE_DIR}/Protocol/Protocol)

aux_source_directory(${PROJECT_SOURCE_DIR}/Protocol/XCode ProtocolSource)
aux_source_directory(${PROJECT_SOURCE_DIR}/Protocol/Protocol ProtocolSource)

include_directories(${PROJECT_SOURCE_DIR}/Logic)
include_directories(${PROJECT_SOURCE_DIR}/Logic/Manager)

aux_source_directory(${PROJECT_SOURCE_DIR}/Logic/Manager LogicSource)

include_directories(${PROJECT_SOURCE_DIR}/DB)
include_directories(${PROJECT_SOURCE_DIR}/DB/Service)
include_directories(${PROJECT_SOURCE_DIR}/DB/Manager)
include_directories(${PROJECT_SOURCE_DIR}/DB/MysqlClient)
include_directories(${PROJECT_SOURCE_DIR}/DB/QueryResult)
include_directories(${PROJECT_SOURCE_DIR}/DB/RedisClient)

aux_source_directory(${PROJECT_SOURCE_DIR}/DB/Service DBSource)
aux_source_directory(${PROJECT_SOURCE_DIR}/DB/Manager DBSource)
aux_source_directory(${PROJECT_SOURCE_DIR}/DB/MysqlClient DBSource)
aux_source_directory(${PROJECT_SOURCE_DIR}/DB/QueryResult DBSource)
aux_source_directory(${PROJECT_SOURCE_DIR}/DB/RedisClient DBSource)
aux_source_directory(${PROJECT_SOURCE_DIR}/DB/Script DataBaseSource)

aux_source_directory(${PROJECT_SOURCE_DIR}/Main SUPER_SRC)

LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/Libs/lib)

#SET(CMAKE_CXX_COMPILER "ccache")
#SET(CMAKE_CXX_COMPILER_ARG1 "g++")
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -std=gnu++11" )

SET(CMAKE_BUILD_TYPE Debug)

ADD_EXECUTABLE(Server ${SUPER_SRC} ${CommonSource} ${ProtocolSource} ${LogicSource} ${DBSource})
TARGET_LINK_LIBRARIES(Server Lua53 protobuf pthread hiredis mysqlclient yassl dl ${MY_ADDLIB})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
