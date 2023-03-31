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
LIST(APPEND PROTO_FLAGS -I${CMAKE_SOURCE_DIR}/bin/proto/c2s)
LIST(APPEND PROTO_FLAGS -I${CMAKE_SOURCE_DIR}/bin/proto/s2s)
LIST(APPEND PROTO_FLAGS -I${CMAKE_SOURCE_DIR}/bin/proto/com)
LIST(APPEND PROTO_FLAGS -I${CMAKE_SOURCE_DIR}/bin/proto/mysql)

#获取需要编译的proto文件
file(GLOB_RECURSE C2S_PROTOS ${CMAKE_SOURCE_DIR}/bin/proto/c2s/*.proto)
file(GLOB_RECURSE S2S_PROTOS ${CMAKE_SOURCE_DIR}/bin/proto/s2s/*.proto)
file(GLOB_RECURSE COM_PROTOS ${CMAKE_SOURCE_DIR}/bin/proto/com/*.proto)
file(GLOB_RECURSE MYSQL_PROTOS ${CMAKE_SOURCE_DIR}/bin/proto/mysql/*.proto)

list(APPEND MSG_PROTOS ${C2S_PROTOS} ${S2S_PROTOS} ${COM_PROTOS} ${MYSQL_PROTOS})
foreach(msg ${MSG_PROTOS})
    message(${msg})
    get_filename_component(FIL_WE ${msg} NAME_WE)
    list(APPEND MESSAGE_SRC "${CMAKE_SOURCE_DIR}/Gen/Message/${FIL_WE}.pb.cc")
    list(APPEND MESSAGE_HDRS "${CMAKE_SOURCE_DIR}Gen/Message/${FIL_WE}.pb.h")

# 生成源码
execute_process(
COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} ${PROTO_FLAGS} --cpp_out=${DST_DIR} ${msg}
)
endforeach()
set_source_files_properties(${MESSAGE_SRC} ${MESSAGE_HDRS} PROPERTIES GENERATED TRUE)