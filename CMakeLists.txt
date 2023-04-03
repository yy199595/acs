cmake_minimum_required(VERSION 3.1.3)

find_program(CLANG_EXECUTABLE clang)
#if(CLANG_EXECUTABLE)
#    message("当前使用编译器为clang")
#    set(CMAKE_C_COMPILER ${CLANG_EXECUTABLE})
#    set(CMAKE_CXX_COMPILER ${CLANG_EXECUTABLE})
#endif()

PROJECT(app)
set(CMAKE_CXX_STANDARD 14)
include(CMake/define.cmake)
include(CMake/function.cmake)

if(__DEBUG__)
    SET(CMAKE_BUILD_TYPE "Debug")
else()
    SET(CMAKE_BUILD_TYPE "Release")
endif()

find_program(PROTOC_EXECUTABLE protoc)
if(NOT PROTOC_EXECUTABLE)
    message("[warning]:请安装protoc来自动生成pb代码")
    message("[info   ]:使用命令 : sudo apt-get install protobuf-compiler")
else()
    include(CMake/proto.cmake)
endif()


message(${CMAKE_BINARY_DIR})

set(ContextPath ${CMAKE_CURRENT_SOURCE_DIR}/Common/Async/Coroutine/Context)
if(WIN32)
    SET(asm_file ${ContextPath}/context_x64.asm)
    if(MSVC)
        if(CMAKE_SIZEOF_VOID_P EQUAL 4)
            set(asm_file ${ContextPath}/context_x86.asm)
        endif()
        set_property(SOURCE ${asm_file} PROPERTY LANGUAGE ASM_MASM)
    else()
        set(asm_file ${ContextPath}/context.S)
    endif()
else()
    SET(asm_file ${ContextPath}/context.S)
endif()
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fexec-charset=GBK")

#第三方库包含
#include_directories(Libs/include/lua)

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
    LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/Libs/mysql/lib)
    # http://cdn.tarsyun.com/src/mysql-connector-c-6.1.11-src.fixed.zip
    #add_subdirectory(${PROJECT_SOURCE_DIR}/Libs/mysql ${PROJECT_SOURCE_DIR}/Libs/bin/mysql)
endif()
#公共组件
add_subdirectory(${PROJECT_SOURCE_DIR}/Libs/lua ${PROJECT_SOURCE_DIR}/Libs/bin/lua)
add_subdirectory(${PROJECT_SOURCE_DIR}/Libs/spdlog ${PROJECT_SOURCE_DIR}/Libs/bin/spdlog)
add_subdirectory(${PROJECT_SOURCE_DIR}/Libs/protobuf/cmake ${PROJECT_SOURCE_DIR}/Libs/bin/protobuf)

#add_subdirectory(${PROJECT_SOURCE_DIR}/Client ${PROJECT_SOURCE_DIRr}/bin)

#LINK_DIRECTORIES(Libs/lib)
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/Libs/bin/lua)
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/Libs/bin/spdlog)
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/Libs/bin/protobuf)


include_directories(${PROJECT_SOURCE_DIR}/Gen)
include_directories(${PROJECT_SOURCE_DIR}/Client)

include_directories(${PROJECT_SOURCE_DIR}/Common)
include_directories(${PROJECT_SOURCE_DIR}/Server)
include_directories(${PROJECT_SOURCE_DIR}/DataBase)

#LINK_DIRECTORIES(Libs/lib)
#SET(CMAKE_BUILD_TYPE "Debug")

find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
    message("使用 ccache 作为编译缓存")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
else()
    message("[warning]:请安装ccache来提高编译速度")
endif()


file(GLOB_RECURSE src_proto "Gen/*.cpp" "Gen/*.cc")
file(GLOB_RECURSE src_common "Common/*.cpp" "Common/*.c")
file(GLOB_RECURSE src_server "Server/*.c" "Server/*.cpp")
file(GLOB_RECURSE src_client "Client/*.c" "Client/*.cpp")
file(GLOB_RECURSE src_database "DataBase/*.cpp" "DataBase/*.c")
list(APPEND build_files ${asm_file} ${src_common} ${src_proto})
list(APPEND build_files ${src_database} ${src_server} ${src_client})

if(MSVC) #创建vs目录

    #file(GLOB_RECURSE file_cmake "CMake/*.cmake")
    file(GLOB_RECURSE file_proto "Gen/*.h" "Gen/*.hpp")
    file(GLOB_RECURSE file_server "Server/*.h" "Server/*.hpp")
    file(GLOB_RECURSE file_client "Client/*.h" "Client/*.hpp")
    file(GLOB_RECURSE file_common "Common/*.h" "Common/*.hpp")
    file(GLOB_RECURSE file_database "DataBase/*.h" "DataBase/*.hpp")
    file(GLOB_RECURSE file_bin "bin/*.json" "bin/*.lua" "bin/*.proto" "bin/*.html")

    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    
    list(APPEND build_files ${file_bin})
    list(APPEND build_files ${file_cmake})
    list(APPEND build_files ${file_common})
    list(APPEND build_files ${file_proto})
    list(APPEND build_files ${file_server})
    list(APPEND build_files ${file_client})
    list(APPEND build_files ${file_database})

    foreach(file_name ${build_files})
        source_group_by_dir(${PROJECT_SOURCE_DIR} ${file_name})
    endforeach()
endif()
ADD_EXECUTABLE(app ${build_files})

list(APPEND static_lib lua)

if(__ENABLE_JEMALLOC__)

    find_package(Jemalloc)
    if(NOT Jemalloc_FOUND)
        message(ERROR "请安装jemalloc 命令:sudo apt-get install libjemalloc-dev")
        #execute_process(COMMAND sudo apt-get install libjemalloc-dev)
    endif()

    list(APPEND static_lib jemalloc)
endif()

if(__ENABLE_MIMALLOC__)
    #list(APPEND static_lib mimalloc)
endif()

if(__ENABLE_OPEN_SSL__)
    list(APPEND static_lib ssl crypto)
endif()

if(__ENABLE_MYSQL__)
    if(WIN32)
        list(APPEND static_lib libmysql mysqlclient)
    elseif(APPLE)
        list(APPEND static_lib mysqlclient yassl)
    else()
        list(APPEND static_lib mysqlclient yassl)
    endif()
endif()


if(WIN32)
    message("当前为win平台")
    add_definitions(-D __OS_WIN__)
    if(MSVC)
        list(APPEND static_lib libprotobufd)
    else()
        list(APPEND static_lib protobufd Ws2_32 Dbghelp Mswsock)
    endif()
elseif(APPLE)
    message("当前为mac平台")
    add_definitions(-D __OS_MAC__)
    list(APPEND static_lib protobufd pthread)
elseif(UNIX)
    message("当前为linux平台")
    add_definitions(-D __OS_LINUX__)

    execute_process(COMMAND uname -r
            OUTPUT_VARIABLE kernel_release
            OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(kernel_release MATCHES ".*-microsoft*")
        message("当前是wsl系统")
        list(APPEND static_lib protobufd pthread dl)
    else()
        list(APPEND static_lib protobufd pthread dl)
    endif()
else()
    message(FATAL_ERROR "未知平台")
endif()

TARGET_LINK_LIBRARIES(app ${static_lib})
SET(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

