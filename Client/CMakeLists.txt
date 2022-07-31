cmake_minimum_required(VERSION 3.1.3)


PROJECT(Client)
set(CMAKE_CXX_STANDARD 11)

message(${CMAKE_BINARY_DIR})

set(ContextPath ${CMAKE_COMMON_DIR}/Common/Coroutine/Context)
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

include_directories(${CMAKE_COMMON_DIR}/Libs/lua/src)
include_directories(${CMAKE_COMMON_DIR}/Libs/protobuf/src)
include_directories(${CMAKE_COMMON_DIR}/Libs/spdlog/include)
include_directories(${CMAKE_COMMON_DIR}/Libs/asio/include)
include_directories(${CMAKE_COMMON_DIR}/Libs/json)
if(__ENABLE_OPEN_SSL__)
    include_directories(${CMAKE_COMMON_DIR}/Libs/include/openssl)
endif()
#公共组件

#LINK_DIRECTORIES(Libs/lib)
LINK_DIRECTORIES(${CMAKE_COMMON_DIR}/Libs/bin/lua)
LINK_DIRECTORIES(${CMAKE_COMMON_DIR}/Libs/bin/spdlog)
LINK_DIRECTORIES(${CMAKE_COMMON_DIR}/Libs/bin/protobuf)

include_directories(${CMAKE_COMMON_DIR}/Common)
include_directories(${CMAKE_COMMON_DIR}/Gen)
include_directories(${CMAKE_COMMON_DIR}/Client)

file(GLOB_RECURSE LogicSource "Client/*.h" "Client/*.cpp" "Client/*.hpp" "Client/*.cc")
file(GLOB_RECURSE CommonSrc "Common/*.h" "Common/*.cpp" "Common/*.hpp" "Common/*.cc" "Common/*.h" "Common/*.c")
file(GLOB_RECURSE ProtocolSrc "Gen/*.h" "Gen/*.cpp" "Gen/*.hpp" "Gen/*.cc")

#LINK_DIRECTORIES(Libs/lib)

if(__DEBUG__)
    SET(CMAKE_BUILD_TYPE "Debug")
else()
    SET(CMAKE_BUILD_TYPE "Release")
endif()

list(APPEND CommonFiles ${CommonSrc} ${ProtocolSrc} ${LogicSource})

find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
    message("使用 ccache 作为编译缓存")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

ADD_EXECUTABLE(Client ${AsmFiles} ${CommonFiles})

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

if(MSVC) #创建vs目录
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    list(APPEND CxxFiles ${AsmFiles} ${CommonFiles})
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
    list(APPEND StaticLibs protobufd pthread)
endif()
TARGET_LINK_LIBRARIES(Client ${StaticLibs})
SET(WORKING_DIRECTORY ${CMAKE_COMMON_DIR}/bin)
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_COMMON_DIR}/bin)

