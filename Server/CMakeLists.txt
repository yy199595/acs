cmake_minimum_required(VERSION 3.1.3)

PROJECT(Server)

#第三方库包含

include_directories(${PROJECT_COMMON_DIR}/Libs/include/rapidjson)
include_directories(${PROJECT_COMMON_DIR}/Libs/include/protobuf)
include_directories(${PROJECT_COMMON_DIR}/Libs/include/asio)
include_directories(${PROJECT_COMMON_DIR}/Libs/include/lua5.3)
include_directories(${PROJECT_COMMON_DIR}/Libs/include/mysql)
include_directories(${PROJECT_COMMON_DIR}/Libs/include/spdlog)
include_directories(${PROJECT_COMMON_DIR}/Libs/include/redis)
include_directories(${PROJECT_COMMON_DIR}/Libs/include/tcmalloc)
#公共组件

include_directories(${PROJECT_SOURCE_DIR}/DB)
include_directories(${PROJECT_SOURCE_DIR}/Logic)
include_directories(${PROJECT_COMMON_DIR}/Common)
include_directories(${PROJECT_COMMON_DIR}/Protocol)
include_directories(${PROJECT_COMMON_DIR}/Common/Network)

include_directories(${PROJECT_SOURCE_DIR}/DB/Component)
include_directories(${PROJECT_SOURCE_DIR}/Logic/Component)
include_directories(${PROJECT_COMMON_DIR}/Common/Component)

file(GLOB_RECURSE DBSource "DB/*.h" "DB/*.cpp" "DB/*.hpp" "DB/*.cc")
file(GLOB_RECURSE MainSource "Main/*.h" "Main/*.cpp" "Main/*.hpp" "Main/*.cc")
file(GLOB_RECURSE LogicSource "Logic/*.h" "Logic/*.cpp" "Logic/*.hpp" "Logic/*.cc")

LINK_DIRECTORIES(${PROJECT_COMMON_DIR}/Libs/lib)

SET(CMAKE_BUILD_TYPE Debug)
list(APPEND ServerFiles ${DBSource} ${MainSource} ${LogicSource})

foreach(file ${CommonFiles})
    message(${file})
endforeach()

foreach(file ${ServerFiles})
    message(${file})
endforeach()

find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
    message("use [ccache]")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif ()

if(MSVC)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    
    foreach(file_name ${AsmFiles})
        source_group_by_dir(${PROJECT_COMMON_DIR} ${file_name})
    endforeach()

    foreach(file_name ${CommonFiles})
        source_group_by_dir(${PROJECT_COMMON_DIR} ${file_name})
    endforeach()

    foreach(file_name ${ServerFiles})
        source_group_by_dir(${PROJECT_SOURCE_DIR} ${file_name})
    endforeach()

    
    ADD_EXECUTABLE(Server ${AsmFiles} ${CommonFiles} ${ServerFiles})
    TARGET_LINK_LIBRARIES(Server Lua53 libprotobufd hiredis libmysql libtcmalloc_minimal)
   
else()
    ADD_EXECUTABLE(Server ${AsmFiles} ${CommonFiles} ${ServerFiles})
    TARGET_LINK_LIBRARIES(Server Lua53 protobuf pthread hiredis mysqlclient yassl dl)
endif()
target_compile_definitions(Server PRIVATE __DEBUG__)
target_compile_definitions(Server PRIVATE __COROUTINE_ASM__)
#target_compile_definitions(Server PRIVATE __THREAD_LOCK__)
message("asm files = " ${AsmFiles})
SET(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

