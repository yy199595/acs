
# openssl

#jemalloc
if(APPLE) #mac上莫名崩溃
    message("在mac上不使用jemalloc")
elseif(EXISTS ${PROJECT_SOURCE_DIR}/Libs/jemalloc/lib/libjemalloc.a)
    message("使用jemalloc作为内存池")
    target_link_libraries(app ${PROJECT_SOURCE_DIR}/Libs/jemalloc/lib/libjemalloc.a)
endif()