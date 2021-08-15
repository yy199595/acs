cmake_minimum_required(VERSION 3.1.3)
project(Sentry)

file(GLOB_RECURSE CommonSrc ${CMAKE_CURRENT_SOURCE_DIR} "Common/*.h" "Common/*.cpp")
file(GLOB_RECURSE ProtocolSrc ${CMAKE_CURRENT_SOURCE_DIR} "Protocol/*.h" "Protocol/*.cpp" "Protocol/*.cc")

list(APPEND CommonFiles ${CommonSrc} ${ProtocolSrc})

set(PROJECT_INIT_PATH ${PROJECT_SOURCE_DIR})

macro(source_group_by_dir dir source_files)
    if(MSVC)
        set(sgbd_cur_dir ${dir})
        foreach(sgbd_file ${source_files})
            #message(${sgbd_cur_dir} " " ${sgbd_file})
            string(REGEX REPLACE ${sgbd_cur_dir}/\(.*\) \\1 sgbd_fpath ${sgbd_file})
            string(REGEX REPLACE "\(.*\)/.*" \\1 sgbd_group_name ${sgbd_fpath})
            string(COMPARE EQUAL ${sgbd_fpath} ${sgbd_group_name} sgbd_nogroup)
            string(REPLACE "/" "\\" sgbd_group_name ${sgbd_group_name})
            if(sgbd_nogroup)
                set(sgbd_group_name "\\")
            endif(sgbd_nogroup)
            source_group(${sgbd_group_name} FILES ${sgbd_file})
            message("group name = " ${sgbd_group_name} "  file = " ${sgbd_file})
        endforeach(sgbd_file)
    endif(MSVC)
endmacro(source_group_by_dir)


add_subdirectory(${PROJECT_SOURCE_DIR}/Server)
add_subdirectory(${PROJECT_SOURCE_DIR}/Client)