cmake_minimum_required(VERSION 3.0)

if (CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-flto)
endif (CMAKE_COMPILER_IS_GNUCXX)

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


find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
    message("使用 ccache 作为编译缓存")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif ()

set(ContextPath ${CMAKE_CURRENT_SOURCE_DIR}/Common/Async/Coroutine/Context)
if (WIN32)
    SET(asm_file ${ContextPath}/context_x64.asm)
    if (MSVC)
        if (CMAKE_SIZEOF_VOID_P EQUAL 4)
            message("============[ x86 ]==========")
            set(asm_file ${ContextPath}/context_x86.asm)
        else ()
            message("============[ x64 ]==========")
        endif ()
        set_property(SOURCE ${asm_file} PROPERTY LANGUAGE ASM_MASM)
    else ()
        set(asm_file ${ContextPath}/context.S)
    endif ()
else ()
    SET(asm_file ${ContextPath}/context.S)
endif ()

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fexec-charset=GBK")

#第三方库包含


include_directories(Libs/protobuf/src)
include_directories(Libs/asio/include)
include_directories(Libs/fmt/include)
include_directories(Libs/lua5.4.6/src)

#公共组件
include_directories(${PROJECT_SOURCE_DIR}/Gen)
include_directories(${PROJECT_SOURCE_DIR}/Robot)

include_directories(${PROJECT_SOURCE_DIR}/Gen/Message)

include_directories(${PROJECT_SOURCE_DIR}/Common)
include_directories(${PROJECT_SOURCE_DIR}/Server)
include_directories(${PROJECT_SOURCE_DIR}/Common/DB)
include_directories(${PROJECT_SOURCE_DIR}/Common/Net)
include_directories(${PROJECT_SOURCE_DIR}/Common/Lua/Src)


if (__ENABLE_OPEN_SSL__)
    include_directories(Libs/openssl/include)
endif ()

add_subdirectory(${PROJECT_SOURCE_DIR}/Libs/lua5.4.6 ${PROJECT_SOURCE_DIR}/Libs/bin/lua)
add_subdirectory(${PROJECT_SOURCE_DIR}/Libs/protobuf/cmake ${PROJECT_SOURCE_DIR}/Libs/bin/protobuf)

file(GLOB_RECURSE src_proto "Gen/*.cpp" "Gen/*.cc")
file(GLOB_RECURSE src_common "Common/*.cpp" "Common/*.c")
file(GLOB_RECURSE src_server "Server/*.c" "Server/*.cpp")
file(GLOB_RECURSE src_client "Robot/*.c" "Robot/*.cpp")

list(APPEND build_files ${asm_file} ${src_common})
list(APPEND build_files ${src_proto} ${src_server} ${src_client})

if (MSVC) #创建vs目录

    #file(GLOB_RECURSE file_md "ReadMe/*.md")
    file(GLOB_RECURSE file_cmake "CMake/*.cmake")
    file(GLOB_RECURSE file_proto "Gen/*.h" "Gen/*.hpp")
    file(GLOB_RECURSE file_server "Server/*.h" "Server/*.hpp")
    file(GLOB_RECURSE file_client "Robot/*.h" "Robot/*.hpp")
    file(GLOB_RECURSE file_common "Common/*.h" "Common/*.hpp")
    #file(GLOB_RECURSE file_lib "Libs/*.h" "Libs/*.hpp" "Libs/*.c" "Libs/*.cpp")
    file(GLOB_RECURSE file_bin "bin/*.csv" "bin/*.json" "bin/*.lua" "bin/*.proto" "bin/*.html")

    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    #list(APPEND build_files ${file_md})
    list(APPEND build_files ${file_bin})
    #list(APPEND build_files ${file_lib})
    list(APPEND build_files ${file_cmake})
    list(APPEND build_files ${file_common})
    list(APPEND build_files ${file_proto})
    list(APPEND build_files ${file_server})
    list(APPEND build_files ${file_client})

    foreach (file_name ${build_files})
        source_group_by_dir(${PROJECT_SOURCE_DIR} ${file_name})
    endforeach ()
endif ()

ADD_EXECUTABLE(app ${build_files})

include(CMake/3rd.cmake)

if (__MEMOEY_CHECK__)
    target_link_options(app PRIVATE -fsanitize=address)
    target_compile_options(app PRIVATE -fsanitize=address)
endif ()


if (WIN32)
    if (MSVC)
        target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/bin/lua/Debug/lua.lib)
        target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/bin/protobuf/Debug/libprotobufd.lib)
        if (__ENABLE_OPEN_SSL__)
            target_link_libraries(app Crypt32)
            target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/openssl/lib/libssl.lib)
            target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/openssl/lib/libcrypto.lib)
        endif ()
    else ()
        #list(APPEND static_lib protobufd Ws2_32 Dbghelp Mswsock)
        target_link_libraries(app Ws2_32 Dbghelp Mswsock)
        target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/bin/protobuf/libprotobufd.a)
        #target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/bin/protobuf/libprotobufd.dll)
    endif ()
elseif (APPLE)
    target_link_libraries(app ${CMAKE_THREAD_LIBS_INIT})

    if (__ENABLE_OPEN_SSL__)
        target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/openssl/lib/libssl.a)
        target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/openssl/lib/libcrypto.a)
    endif ()
    target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/bin/lua/liblua.a)
    target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/bin/protobuf/libprotobufd.a)
elseif (UNIX)
    target_link_libraries(app pthread dl)
    if (__ENABLE_OPEN_SSL__)
        message("链接openssl库")
        target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/openssl/lib/libssl.a)
        target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/openssl/lib/libcrypto.a)
    endif ()
    target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/bin/lua/liblua.a)
    if (EXISTS ${PROJECT_SOURCE_DIR}/Libs/bin/protobuf/libprotobufd.a)
        target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/bin/protobuf/libprotobufd.a)
    else ()
        target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/bin/protobuf/libprotobuf.a)
    endif ()
endif ()

SET(WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

