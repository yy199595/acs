cmake_minimum_required(VERSION 3.0)

project(lua)

if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
    add_definitions(-DLUA_USE_LINUX)
    list(APPEND LINK_LIBRARIES readline dl m)
else ()
    add_definitions(-DLUA_USE_MACOSX)
    list(APPEND LINK_LIBRARIES readline)
endif ()


SET(LUA_SOURCE_FILES
        ./src/lapi.c
        ./src/lauxlib.c
        ./src/lbaselib.c
        ./src/lcode.c
        ./src/lcorolib.c
        ./src/lctype.c
        ./src/ldblib.c
        ./src/ldebug.c
        ./src/ldo.c
        ./src/ldump.c
        ./src/lfunc.c
        ./src/lgc.c
        ./src/linit.c
        ./src/liolib.c
        ./src/llex.c
        ./src/lmathlib.c
        ./src/lmem.c
        ./src/loadlib.c
        ./src/lobject.c
        ./src/lopcodes.c
        ./src/loslib.c
        ./src/lparser.c
        ./src/lstate.c
        ./src/lstring.c
        ./src/lstrlib.c
        ./src/ltable.c
        ./src/ltablib.c
        ./src/ltm.c
        ./src/lundump.c
        ./src/lutf8lib.c
        ./src/lvm.c
        ./src/lzio.c
)

add_library(lua-share SHARED ${LUA_SOURCE_FILES})
add_library(lua-static STATIC ${LUA_SOURCE_FILES})

set_target_properties(lua-share PROPERTIES OUTPUT_NAME lua)
set_target_properties(lua-static PROPERTIES OUTPUT_NAME lua)

if (WIN32)

else ()
    target_compile_options(lua-share PUBLIC -fPIC)
endif (WIN32)