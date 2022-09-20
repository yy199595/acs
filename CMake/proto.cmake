find_package(Protobuf 3 REQUIRED)

#设置输出路径
set(MESSAGE_DIR ${CMAKE_SOURCE_DIR}/Gen/Message)
if(EXISTS "${CMAKE_SOURCE_DIR}/Gen/Message" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/Gen/Message")
    SET(DST_DIR ${MESSAGE_DIR})
else()
    file(MAKE_DIRECTORY ${MESSAGE_DIR})
    SET(DST_DIR ${MESSAGE_DIR})
endif()

set(MESSAGE_SRC "")
set(MESSAGE_HDRS "")
#设置protoc的搜索路径
LIST(APPEND PROTO_FLAGS -I${CMAKE_SOURCE_DIR}/bin/proto/message)
LIST(APPEND PROTO_FLAGS -I${CMAKE_SOURCE_DIR}/bin/proto/mysql)

#获取需要编译的proto文件
file(GLOB_RECURSE MSG_PROTOS ${CMAKE_SOURCE_DIR}/bin/proto/message/*.proto)
file(GLOB_RECURSE MYSQL_PROTOS ${CMAKE_SOURCE_DIR}/bin/proto/mysql/*.proto)

list(APPEND ${MSG_PROTOS} ${MYSQL_PROTOS})
foreach(msg ${MSG_PROTOS})
    get_filename_component(FIL_WE ${msg} NAME_WE)
    message(${FIL_WE})
    list(APPEND MESSAGE_SRC "${CMAKE_SOURCE_DIR}/Gen/Message/${FIL_WE}.pb.cc")
    list(APPEND MESSAGE_HDRS "${CMAKE_SOURCE_DIR}Gen/Message/${FIL_WE}.pb.h")

# 生成源码
execute_process(
COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} ${PROTO_FLAGS} --cpp_out=${DST_DIR} ${msg}
)
endforeach()
set_source_files_properties(${MESSAGE_SRC} ${MESSAGE_HDRS} PROPERTIES GENERATED TRUE)