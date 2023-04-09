if(MSVC)
    enable_language(C CXX ASM_MASM)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
else()
    enable_language(C CXX ASM)
endif()

add_definitions(-w) #忽略警告
add_definitions(-D ASIO_STANDALONE)


#option(protobuf_BUILD_TESTS OFF)
#option(protobuf_BUILD_EXAMPLES OFF)
#option(protobuf_BUILD_CONFORMANCE OFF)
#option(protobuf_BUILD_SHARED_LIBS ON)
option(protobuf_BUILD_TESTS OFF)
option(protobuf_BUILD_EXAMPLES OFF)
option(protobuf_BUILD_CONFORMANCE OFF)
if(WIN32 AND NOT MSVC)
    #set(protobuf_BUILD_SHARED_LIBS ON) #编译动态库
endif()
option(__DEBUG__ "debug模式" ON)

option(__DEBUG_STACK__ "开启堆栈打印" ON)
option(__HTTP_DEBUG_LOG__ "打印http日志" OFF)
option(__REDIS_DEBUG__ "开始redis调试" OFF)
option(ONLY_MAIN_THREAD "启用单线程模式" OFF)
option(__NET_ERROR_LOG__ "打印网络层错误" ON)
option(__ENABLE_REDIS__ "是否使用redis" ON)

option(__ENABLE_MONGODB__ "是否使用mongodb" ON)

set(CMAKE_COMMON_DIR ${PROJECT_SOURCE_DIR})



if(ONLY_MAIN_THREAD)
    message("当前网络为单线程模型")
    add_definitions(-D ONLY_MAIN_THREAD)
else()
    message("当前网络为多线程模型")
endif()

if(__ENABLE_REDIS__)
    message("启用redis数据库")
    add_definitions(-D __ENABLE_REDIS__)
endif()

if(__ENABLE_MONGODB__)
    message("启用mongodb数据库")
    add_definitions(-D __ENABLE_MONGODB__)
endif()

if(__DEBUG__)
    message("当前为debug模式")
    add_definitions(-D __DEBUG__)

    if(__REDIS_DEBUG__)
        message("打印redis命令到控制台")
        add_definitions(-D __REDIS_DEBUG__)
    endif()
endif()

if(WIN32)
    message("当前为win平台")
    add_definitions(-D __OS_WIN__)
    if(MSVC)
        add_definitions(-D __OS_WIN_MSVC_)
    endif()
    add_definitions(-D NOMINMAX)
    remove_definitions(GetMessage)
    add_definitions(-D WIN32_LEAN_AND_MEAN)
elseif(APPLE)
    message("当前为mac平台")
    add_definitions(-D __OS_MAC__)
elseif(UNIX)
    message("当前为linux平台")
    add_definitions(-D __OS_LINUX__)
endif()

if(MSVC)
    add_compile_options(/W4 /fp:fast /EHsc)
    add_link_options(/SAFESEH:NO)
else()
#- - wall:启用所有警告。
#- -g:输出调试信息。
#- - wno -sign-compare:禁用与符号比较相关的警告。
#- - wno -class-memaccess:禁用类成员访问相关的警告。
#- - wno -strict-aliasing:禁用严格混叠相关的警告。
    add_compile_options(-Wall -g -Wno-sign-compare -Wno-class-memaccess -Wno-strict-aliasing)
    if(APPLE)
        add_compile_options(-fno-pie)
    endif()
endif()

if(__OS_LINUX__)
    find_program(CCACHE_PROGRAM ccache)
    if (CCACHE_PROGRAM)
        message("使用ccache作为编译缓存")
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
    else()
        message("[warning]:请安装ccache来提高编译速度")
    endif()
endif()

find_program(PROTOC_EXECUTABLE protoc)
if(NOT PROTOC_EXECUTABLE)
    message("[warning]:请安装protoc来自动生成pb代码")
    message("[info   ]:使用命令 : sudo apt-get install protobuf-compiler")
else()
    include(CMake/proto.cmake)
endif()
