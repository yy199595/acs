cmake_minimum_required(VERSION 3.1.3)
PROJECT(Client)


#第三方库包含

include_directories(${PROJECT_SOURCE_DIR}/../Libs/include/rapidjson)
include_directories(${PROJECT_SOURCE_DIR}/../Libs/include/protobuf)
include_directories(${PROJECT_SOURCE_DIR}/../Libs/include/asio)
include_directories(${PROJECT_SOURCE_DIR}/../Libs/include/lua5.3)
include_directories(${PROJECT_SOURCE_DIR}/../Libs/include/spdlog)
#公共组件

include_directories(${PROJECT_SOURCE_DIR}/../Common)
include_directories(${PROJECT_SOURCE_DIR}/../Common/Pool)
include_directories(${PROJECT_SOURCE_DIR}/../Common/Core)
include_directories(${PROJECT_SOURCE_DIR}/../Common/Coroutine)
include_directories(${PROJECT_SOURCE_DIR}/../Common/Define)
include_directories(${PROJECT_SOURCE_DIR}/../Common/Global)
include_directories(${PROJECT_SOURCE_DIR}/../Common/Manager)
include_directories(${PROJECT_SOURCE_DIR}/../Common/Module)
include_directories(${PROJECT_SOURCE_DIR}/../Common/NetWork)
include_directories(${PROJECT_SOURCE_DIR}/../Common/Object)
include_directories(${PROJECT_SOURCE_DIR}/../Common/Other)
include_directories(${PROJECT_SOURCE_DIR}/../Common/Thread)
include_directories(${PROJECT_SOURCE_DIR}/../Common/Timer)
include_directories(${PROJECT_SOURCE_DIR}/../Common/Util)
include_directories(${PROJECT_SOURCE_DIR}/../Common/Script)
include_directories(${PROJECT_SOURCE_DIR}/../Common/Service)

aux_source_directory(${PROJECT_SOURCE_DIR}/../Common/Core SUPER_SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/../Common/Pool SUPER_SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/../Common/Coroutine SUPER_SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/../Common/Define SUPER_SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/../Common/Global SUPER_SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/../Common/Manager SUPER_SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/../Common/Module SUPER_SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/../Common/NetWork SUPER_SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/../Common/Object SUPER_SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/../Common/Other SUPER_SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/../Common/Thread SUPER_SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/../Common/Timer SUPER_SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/../Common/Util SUPER_SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/../Common/Script SUPER_SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/../Common/Service SUPER_SRC)

include_directories(${PROJECT_SOURCE_DIR}/../Protocol)
include_directories(${PROJECT_SOURCE_DIR}/../Protocol/XCode)
include_directories(${PROJECT_SOURCE_DIR}/../Protocol/Protocol)

aux_source_directory(${PROJECT_SOURCE_DIR}/../Protocol/XCode SUPER_SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/../Protocol/Protocol SUPER_SRC)

include_directories(${PROJECT_SOURCE_DIR}/Manager)
aux_source_directory(${PROJECT_SOURCE_DIR}/Manager SUPER_SRC)

aux_source_directory(${PROJECT_SOURCE_DIR}/Main SUPER_SRC)
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/../Libs/lib)

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_BUILD_TYPE Debug)

ADD_EXECUTABLE(Client ${SUPER_SRC})
TARGET_LINK_LIBRARIES(Client Lua53 protobuf pthread dl ${MY_ADDLIB})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)



