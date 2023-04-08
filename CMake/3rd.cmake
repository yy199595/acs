
# openssl
if (EXISTS ${PROJECT_SOURCE_DIR}/Libs/openssl/libssl.a)
    message("启用ssl")
    #add_definitions(-D __ENABLE_OPEN_SSL__)
    target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/openssl/libssl.a)
    target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/openssl/libcrypto.a)
endif ()

#jemalloc
if(NOT __OS_MAC__) #mac上莫名崩溃
    if(EXISTS ${PROJECT_SOURCE_DIR}/Libs/jemalloc/lib/libjemalloc.a)
        message("使用jemalloc作为内存池")
        target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/jemalloc/lib/libjemalloc.a)
    endif()
endif()