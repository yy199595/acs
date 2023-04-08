cmake_minimum_required(VERSION 3.1.3)

find_program(CLANG_EXECUTABLE clang)
#if(CLANG_EXECUTABLE)
#    message("当前使用编译器为clang")
#    set(CMAKE_C_COMPILER ${CLANG_EXECUTABLE})
#    set(CMAKE_CXX_COMPILER ${CLANG_EXECUTABLE})
#endif()

PROJECT(app)
set(CMAKE_CXX_STANDARD 14)

include(CMake/url.cmake)
include(CMake/code.cmake)
include(CMake/define.cmake)
include(CMake/function.cmake)

if(__DEBUG__)
    SET(CMAKE_BUILD_TYPE "Debug")
else()
    SET(CMAKE_BUILD_TYPE "Release")
endif()

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
include_directories(Libs/json)
include_directories(Libs/lua/src)
include_directories(Libs/protobuf/src)
include_directories(Libs/spdlog/include)
include_directories(Libs/asio/include)
include_directories(Libs/mysql/include)
include_directories(Libs/openssl/include)

#公共组件
include_directories(${PROJECT_SOURCE_DIR}/Gen)
include_directories(${PROJECT_SOURCE_DIR}/Client)

include_directories(${PROJECT_SOURCE_DIR}/Common)
include_directories(${PROJECT_SOURCE_DIR}/Server)
include_directories(${PROJECT_SOURCE_DIR}/DataBase)

add_subdirectory(${PROJECT_SOURCE_DIR}/Libs/lua ${PROJECT_SOURCE_DIR}/Libs/bin/lua)
add_subdirectory(${PROJECT_SOURCE_DIR}/Libs/spdlog ${PROJECT_SOURCE_DIR}/Libs/bin/spdlog)
add_subdirectory(${PROJECT_SOURCE_DIR}/Libs/protobuf/cmake ${PROJECT_SOURCE_DIR}/Libs/bin/protobuf)


file(GLOB_RECURSE src_proto "Gen/*.cpp" "Gen/*.cc")
file(GLOB_RECURSE src_common "Common/*.cpp" "Common/*.c")
file(GLOB_RECURSE src_server "Server/*.c" "Server/*.cpp")
file(GLOB_RECURSE src_client "Client/*.c" "Client/*.cpp")
file(GLOB_RECURSE src_database "DataBase/*.cpp" "DataBase/*.c")
list(APPEND build_files ${asm_file} ${src_common} ${src_proto})
list(APPEND build_files ${src_database} ${src_server} ${src_client})

if(MSVC) #创建vs目录

    file(GLOB_RECURSE file_cmake "CMake/*.cmake")
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

include(CMake/3rd.cmake)
include(CMake/mysql.cmake) #add_lib 必须在add_exe之后

if(WIN32)
    if(MSVC)
        target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/bin/lua/lua.lib)
        target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/bin/protobuf/libprotobufd.lib)
    else()
        #list(APPEND static_lib protobufd Ws2_32 Dbghelp Mswsock)
        target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/bin/lua/liblua.a)
        target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/bin/protobuf/libprotobufd.a)
    endif()
elseif(APPLE)
    target_link_libraries(app ${CMAKE_THREAD_LIBS_INIT})
    target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/bin/lua/liblua.a)
    target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/bin/protobuf/libprotobufd.a)
elseif(UNIX)
    target_link_libraries(app ${CMAKE_THREAD_LIBS_INIT})
    target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/bin/lua/liblua.a)
    target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/bin/protobuf/libprotobufd.a)
endif()

SET(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

