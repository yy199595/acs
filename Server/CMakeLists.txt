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

file(GLOB_RECURSE DBSource "DB/*.h" "DB/*.cpp" "DB/*.hpp" "DB/*.cc")
file(GLOB_RECURSE MainSource "Main/*.h" "Main/*.cpp" "Main/*.hpp" "Main/*.cc")
file(GLOB_RECURSE LogicSource "Logic/*.h" "Logic/*.cpp" "Logic/*.hpp" "Logic/*.cc")

LINK_DIRECTORIES(${PROJECT_INIT_PATH}/Libs/lib)

SET(CMAKE_BUILD_TYPE Debug)
list(APPEND ServerFiles ${DBSource} ${MainSource} ${LogicSource})

if(MSVC)
    message("currentdir = " ${PROJECT_SOURCE_DIR} " last dir = " ${PROJECT_INIT_PATH})
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    
    foreach(file_name ${CommonFiles})
        source_group_by_dir(${PROJECT_INIT_PATH} ${file_name})
    endforeach()

    foreach(file_name ${ServerFiles})
        source_group_by_dir(${PROJECT_SOURCE_DIR} ${file_name})
    endforeach()

    ADD_EXECUTABLE(Server ${AsmFiles} ${CommonFiles} ${ServerFiles})
    TARGET_LINK_LIBRARIES(Server Lua53 libprotobufd hiredis libmysql)
else()
    ADD_EXECUTABLE(Server ${AsmFiles} ${CommonFiles} ${ServerFiles})
    TARGET_LINK_LIBRARIES(Server Lua53 protobuf pthread hiredis mysqlclient yassl dl)
endif()
message("asm files = " ${AsmFiles})
SET(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

