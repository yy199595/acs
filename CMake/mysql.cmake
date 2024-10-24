
set(mysql_lib_dir ${PROJECT_SOURCE_DIR}/Libs/mysql/lib)
if(NOT EXISTS ${mysql_lib_dir})
    file(MAKE_DIRECTORY "${mysql_lib_dir}")
endif()

if(MSVC)
    set(mysql_target_ssl_path ${PROJECT_SOURCE_DIR}/Libs/mysql/lib/libmysql.lib)
    set(mysql_target_client_path ${PROJECT_SOURCE_DIR}/Libs/mysql/lib/mysqlclient.lib)
    if(NOT EXISTS ${mysql_target_ssl_path})
        message("please copy libmysql.lib to " ${mysql_lib_dir})
    endif()

    if(NOT EXISTS ${mysql_target_client_path})
        message("please copy mysqlclient.lib to " ${mysql_lib_dir})
    endif()
else()
    set(mysql_source_ssl_path ${PROJECT_SOURCE_DIR}/Libs/bin/mysql-connector-c-6.1.11-src/extra/yassl/libyassl.a)
    set(mysql_source_client_path ${PROJECT_SOURCE_DIR}/Libs/bin/mysql-connector-c-6.1.11-src/libmysql/libmysqlclient.a)
    set(mysql_target_ssl_path ${PROJECT_SOURCE_DIR}/Libs/mysql/lib/libyassl.lib)
    set(mysql_target_client_path ${PROJECT_SOURCE_DIR}/Libs/mysql/lib/libclientlib.lib)
endif()

if(UNIX)
    if(NOT EXISTS ${mysql_source_ssl_path})
        message("please invoke [./build.sh mysql]")
    else(NOT EXISTS ${mysql_target_ssl_path})
        file(COPY "${mysql_source_ssl_path}" DESTINATION "${mysql_lib_dir}")
        file(COPY "${mysql_source_client_path}" DESTINATION "${mysql_lib_dir}")
    endif()
    #LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/Libs/mysql/lib)
endif()


if(MSVC)
    if(EXISTS ${PROJECT_SOURCE_DIR}/Libs/mysql/lib/libmysql.lib)
        if(EXISTS ${PROJECT_SOURCE_DIR}/Libs/mysql/lib/mysqlclient.lib)
            message("MSVC启用mysql客户端")
            add_definitions(-D __ENABLE_MYSQL__)
            target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/mysql/lib/libmysql.lib)
            target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/mysql/lib/mysqlclient.lib)
        endif()
    endif()
else()
    if(EXISTS ${PROJECT_SOURCE_DIR}/Libs/mysql/lib/libyassl.a)
        if(EXISTS ${PROJECT_SOURCE_DIR}/Libs/mysql/lib/libmysqlclient.a)
            message("GCC启用mysql客户端")
            add_definitions(-D __ENABLE_MYSQL__)
            if(__ENABLE_OPEN_SSL__)
                message("mysql使用openssl")
            else ()
                target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/mysql/lib/libyassl.a)
            endif ()
            target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/mysql/lib/libmysqlclient.a)
        endif()
    endif()
endif()