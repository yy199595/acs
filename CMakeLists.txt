cmake_minimum_required(VERSION 3.1.3)


PROJECT(Server)
set(CMAKE_CXX_STANDARD 11)
include(CMake/define.cmake)
include(CMake/proto.cmake)
if(WIN32)

else()
    include(CMake/function.cmake)
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
include_directories(Libs/include/lua)
include_directories(Libs/include/asio)
include_directories(Libs/include/spdlog)
include_directories(Libs/include/protobuf)
include_directories(Libs/include/rapidjson)
if(__ENABLE_OPEN_SSL__)
    include_directories(Libs/include/openssl)
endif()
#公共组件

LINK_DIRECTORIES(Libs/lib)


include_directories(Server/Gate)
include_directories(Server/Logic)
include_directories(${PROJECT_SOURCE_DIR}/Common)
include_directories(${PROJECT_SOURCE_DIR}/Gen)
include_directories(${PROJECT_SOURCE_DIR}/Server)

if (__ENABLE_CLIENT__)
    include_directories(${PROJECT_SOURCE_DIR}/Client)
    file(GLOB_RECURSE LogicSource "Client/*.h" "Client/*.cpp" "Client/*.hpp" "Client/*.cc")
endif()

file(GLOB_RECURSE CommonSrc "Common/*.h" "Common/*.cpp" "Common/*.hpp" "Common/*.cc" "Common/*.h" "Common/*.c")
file(GLOB_RECURSE ProtocolSrc "Gen/*.h" "Gen/*.cpp" "Gen/*.hpp" "Gen/*.cc")

file(GLOB_RECURSE GateSource "Server/*.h" "Server/*.cpp" "Server/*.hpp" "Server/*.cc")
file(GLOB_RECURSE MainSource "Main/*.h" "Main/*.cpp" "Main/*.hpp" "Main/*.cc")

LINK_DIRECTORIES(Libs/lib)

if(__DEBUG__)
    SET(CMAKE_BUILD_TYPE "Debug")
else()
    SET(CMAKE_BUILD_TYPE "Release")
endif()

list(APPEND CommonFiles ${CommonSrc} ${ProtocolSrc})
list(APPEND ServerFiles ${MainSource} ${LogicSource} ${GateSource})

find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
    message("使用 ccache 作为编译缓存")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

ADD_EXECUTABLE(Sentry ${AsmFiles} ${CommonFiles} ${ServerFiles})

list(APPEND StaticLibs lua)

if(__ENABLE_JEMALLOC__)
    list(APPEND StaticLibs jemalloc)
endif()

if(__ENABLE_MIMALLOC__)
    list(APPEND StaticLibs mimalloc)
endif()

if(__ENABLE_OPEN_SSL__)
    list(APPEND StaticLibs ssl crypto)
endif()

if(MSVC) #创建vs目录
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    list(APPEND CxxFiles ${AsmFiles} ${CommonFiles} ${ServerFiles})
    foreach(file_name ${CxxFiles})
        source_group_by_dir(${PROJECT_SOURCE_DIR} ${file_name})
    endforeach()
endif()

if(WIN32)
    message("当前为win平台")
    add_definitions(-D __OS_WIN__)
    list(APPEND StaticLibs libprotobufd)
elseif(APPLE)
    message("当前为mac平台")
    add_definitions(-D __OS_MAC__)
    list(APPEND StaticLibs protobuf pthread)
else()
    message("当前为linux平台")
    add_definitions(-D __OS_LINUX__)
    list(APPEND StaticLibs protobuf pthread dl)
endif()
TARGET_LINK_LIBRARIES(Sentry ${StaticLibs})
SET(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

