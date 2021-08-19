cmake_minimum_required(VERSION 3.1.3)

PROJECT(Server)

#第三方库包含

include_directories(${PROJECT_INIT_PATH}/Libs/include/rapidjson)
include_directories(${PROJECT_INIT_PATH}/Libs/include/protobuf)
include_directories(${PROJECT_INIT_PATH}/Libs/include/asio)
include_directories(${PROJECT_INIT_PATH}/Libs/include/lua5.3)
include_directories(${PROJECT_INIT_PATH}/Libs/include/mysql)
include_directories(${PROJECT_INIT_PATH}/Libs/include/spdlog)
include_directories(${PROJECT_INIT_PATH}/Libs/include/redis)
#公共组件

include_directories(${PROJECT_SOURCE_DIR}/DB)
include_directories(${PROJECT_SOURCE_DIR}/Logic)
include_directories(${PROJECT_INIT_PATH}/Common)
include_directories(${PROJECT_INIT_PATH}/Protocol)

#框架底层
aux_source_directory(${PROJECT_INIT_PATH}/Common/Core CommonSource)
aux_source_directory(${PROJECT_INIT_PATH}/Common/Coroutine CommonSource)
aux_source_directory(${PROJECT_INIT_PATH}/Common/Define CommonSource)
aux_source_directory(${PROJECT_INIT_PATH}/Common/Global CommonSource)
aux_source_directory(${PROJECT_INIT_PATH}/Common/Manager CommonSource)
aux_source_directory(${PROJECT_INIT_PATH}/Common/Module CommonSource)
aux_source_directory(${PROJECT_INIT_PATH}/Common/NetWork CommonSource)
aux_source_directory(${PROJECT_INIT_PATH}/Common/Object CommonSource)
aux_source_directory(${PROJECT_INIT_PATH}/Common/Other CommonSource)
aux_source_directory(${PROJECT_INIT_PATH}/Common/Thread CommonSource)
aux_source_directory(${PROJECT_INIT_PATH}/Common/Timer CommonSource)
aux_source_directory(${PROJECT_INIT_PATH}/Common/Util CommonSource)
aux_source_directory(${PROJECT_INIT_PATH}/Common/Script CommonSource)
aux_source_directory(${PROJECT_INIT_PATH}/Common/Service CommonSource)
aux_source_directory(${PROJECT_INIT_PATH}/Common/Pool CommonSource)


aux_source_directory(${PROJECT_INIT_PATH}/Protocol/XCode ProtocolSource)
aux_source_directory(${PROJECT_INIT_PATH}/Protocol/Protocol ProtocolSource)

aux_source_directory(${PROJECT_SOURCE_DIR}/Logic/Manager LogicSource)

aux_source_directory(${PROJECT_SOURCE_DIR}/DB/Service DBSource)
aux_source_directory(${PROJECT_SOURCE_DIR}/DB/Manager DBSource)
aux_source_directory(${PROJECT_SOURCE_DIR}/DB/MysqlClient DBSource)
aux_source_directory(${PROJECT_SOURCE_DIR}/DB/QueryResult DBSource)
aux_source_directory(${PROJECT_SOURCE_DIR}/DB/RedisClient DBSource)
aux_source_directory(${PROJECT_SOURCE_DIR}/DB/Script DataBaseSource)

aux_source_directory(${PROJECT_SOURCE_DIR}/Main MainSource)

LINK_DIRECTORIES(${PROJECT_INIT_PATH}/Libs/lib)

#SET(CMAKE_CXX_COMPILER "ccache")
#SET(CMAKE_CXX_COMPILER_ARG1 "g++")
SET(CMAKE_CXX_STANDARD 11)
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -std=gnu++11" )


SET(CMAKE_BUILD_TYPE Debug)


if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    message("currentdir = " ${PROJECT_SOURCE_DIR} " last dir = " ${PROJECT_INIT_PATH})
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    file(REMOVE_RECURSE ${PROJECT_SOURCE_DIR}/CMakeFiles)
    file(GLOB_RECURSE ServerFiles ${CMAKE_CURRENT_SOURCE_DIR} "*.h" "*.cpp")
    
    #list(APPEND ServerFiles CommonFiles files)
    foreach(file_name ${CommonFiles})
        source_group_by_dir(${PROJECT_INIT_PATH} ${file_name})
    endforeach()

    foreach(file_name ${ServerFiles})
        source_group_by_dir(${PROJECT_SOURCE_DIR} ${file_name})
    endforeach()

    ADD_EXECUTABLE(Server ${AsmFiles} ${CommonFiles} ${ServerFiles})
    TARGET_LINK_LIBRARIES(Server Lua53 libprotobufd hiredis libmysql)
else()
    ADD_EXECUTABLE(Server ${AsmFiles} ${MainSource} ${CommonSource} ${ProtocolSource} ${LogicSource} ${DBSource})
    TARGET_LINK_LIBRARIES(Server Lua53 protobuf pthread hiredis mysqlclient yassl dl)
endif()
message("asm files = " ${AsmFiles})
SET(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

