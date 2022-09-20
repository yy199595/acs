cmake_minimum_required(VERSION 3.1.3)

PROJECT(app)
set(CMAKE_CXX_STANDARD 11)
include(CMake/define.cmake)
include(CMake/function.cmake)

if(WIN32)

else()
    include(CMake/proto.cmake)
endif()
message(${CMAKE_BINARY_DIR})

set(ContextPath ${CMAKE_CURRENT_SOURCE_DIR}/Common/Coroutine/Context)
if(WIN32)
    SET(AsmFiles ${ContextPath}/context_x64.asm)
    if(MSVC)
        if(CMAKE_SIZEOF_VOID_P EQUAL 4)
            set(AsmFiles ${ContextPath}/context_x86.asm)
        endif()
            set_property(SOURCE ${AsmFiles} PROPERTY LANGUAGE ASM_MASM)
    else()
        set(AsmFiles ${ContextPath}/context.S)
    endif()
else()
    SET(AsmFiles ${ContextPath}/context.S)
endif()
MESSAGE(${AsmFiles})

#第三方库包含
#include_directories(Libs/include/lua)

include_directories(Libs/amy)
include_directories(Libs/lua/src)
include_directories(Libs/protobuf/src)
include_directories(Libs/spdlog/include)
include_directories(Libs/asio/include)

include_directories(Libs/json)
if(__ENABLE_OPEN_SSL__)
    include_directories(Libs/include/openssl)
endif()
if(__ENABLE_MYSQL__)
    include_directories(Libs/mysql/include)
endif()
#公共组件
add_subdirectory(${PROJECT_SOURCE_DIR}/Libs/lua ${PROJECT_SOURCE_DIR}/Libs/bin/lua)
add_subdirectory(${PROJECT_SOURCE_DIR}/Libs/spdlog ${PROJECT_SOURCE_DIR}/Libs/bin/spdlog)
add_subdirectory(${PROJECT_SOURCE_DIR}/Libs/protobuf/cmake ${PROJECT_SOURCE_DIR}/Libs/bin/protobuf)

#add_subdirectory(${PROJECT_SOURCE_DIR}/Client ${PROJECT_SOURCE_DIRr}/bin)

#LINK_DIRECTORIES(Libs/lib)
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/Libs/bin/lua)
if(__ENABLE_MYSQL__)
    LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/Libs/mysql/lib)
endif()
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/Libs/bin/spdlog)
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/Libs/bin/protobuf)


include_directories(${PROJECT_SOURCE_DIR}/Gen)
include_directories(${PROJECT_SOURCE_DIR}/Common)
include_directories(${PROJECT_SOURCE_DIR}/Server)
include_directories(${PROJECT_SOURCE_DIR}/Client)

if(__ENABLE_MONGODB__)
    include_directories(${PROJECT_SOURCE_DIR}/DataBase/Mongo)
    file(GLOB_RECURSE MongoFiles "DataBase/Mongo/*.h" "DataBase/Mongo/*.cpp")
endif()

if(__ENABLE_MYSQL__)
    include_directories(${PROJECT_SOURCE_DIR}/DataBase/Mysql)
    file(GLOB_RECURSE MysqlFiles "DataBase/Mysql/*.h" "DataBase/Mysql/*.cpp")
endif()

file(GLOB_RECURSE CommonSrc "Common/*.h" "Common/*.cpp")
file(GLOB_RECURSE ProtocolSrc "Gen/*.h" "Gen/*.cpp" "Gen/*.hpp" "Gen/*.cc")

file(GLOB_RECURSE ServerSource "Server/*.h" "Server/*.cpp")
file(GLOB_RECURSE ClientSource "Client/*.h" "Client/*.cpp")

#LINK_DIRECTORIES(Libs/lib)

if(__DEBUG__)
    SET(CMAKE_BUILD_TYPE "Debug")
else()
    SET(CMAKE_BUILD_TYPE "Release")
endif()

list(APPEND CommonFiles ${CommonSrc} ${ProtocolSrc} ${MongoFiles} ${MysqlFiles})
list(APPEND ServerFiles ${MainSource} ${LogicSource} ${ServerSource})

find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
    message("使用 ccache 作为编译缓存")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

ADD_EXECUTABLE(app ${AsmFiles} ${CommonFiles} ${ServerFiles} ${ClientSource})

list(APPEND StaticLibs lua)

if(__ENABLE_JEMALLOC__)
    list(APPEND StaticLibs jemalloc)
endif()

if(__ENABLE_MIMALLOC__)
    #list(APPEND StaticLibs mimalloc)
endif()

if(__ENABLE_OPEN_SSL__)
    list(APPEND StaticLibs ssl crypto)
endif()

if(__ENABLE_MYSQL__)
    if(WIN32)
        list(APPEND StaticLibs libmysql mysqlclient)
    elseif(APPLE)
        list(APPEND StaticLibs mysqlclient yassl taocrypt)
    else()
        list(APPEND StaticLibs mysqlclient yassl taocrypt dl)
    endif()
endif()


if(MSVC) #创建vs目录
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    list(APPEND CxxFiles ${AsmFiles} ${CommonFiles} ${ServerFiles} ${ClientSource})
    foreach(file_name ${CxxFiles})
        source_group_by_dir(${PROJECT_SOURCE_DIR} ${file_name})
    endforeach()
endif()

if(WIN32)
    message("当前为win平台")
    add_definitions(-D __OS_WIN__)
    list(APPEND StaticLibs libprotobufd spdlogd)
elseif(APPLE)
    message("当前为mac平台")
    add_definitions(-D __OS_MAC__)
    list(APPEND StaticLibs protobufd spdlogd pthread)
else()
    message("当前为linux平台")
    add_definitions(-D __OS_LINUX__)
    list(APPEND StaticLibs protobuf pthread)
endif()
TARGET_LINK_LIBRARIES(app ${StaticLibs})
SET(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

