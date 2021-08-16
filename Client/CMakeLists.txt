cmake_minimum_required(VERSION 3.1.3)
PROJECT(Client)


#第三方库包含

include_directories(${PROJECT_INIT_PATH}/Libs/include/rapidjson)
include_directories(${PROJECT_INIT_PATH}/Libs/include/protobuf)
include_directories(${PROJECT_INIT_PATH}/Libs/include/asio)
include_directories(${PROJECT_INIT_PATH}/Libs/include/lua5.3)
include_directories(${PROJECT_INIT_PATH}/Libs/include/spdlog)
#公共组件

include_directories(${PROJECT_INIT_PATH}/Common)


aux_source_directory(${PROJECT_INIT_PATH}/Common/Core SUPER_SRC)
aux_source_directory(${PROJECT_INIT_PATH}/Common/Pool SUPER_SRC)
aux_source_directory(${PROJECT_INIT_PATH}/Common/Coroutine SUPER_SRC)
aux_source_directory(${PROJECT_INIT_PATH}/Common/Define SUPER_SRC)
aux_source_directory(${PROJECT_INIT_PATH}/Common/Global SUPER_SRC)
aux_source_directory(${PROJECT_INIT_PATH}/Common/Manager SUPER_SRC)
aux_source_directory(${PROJECT_INIT_PATH}/Common/Module SUPER_SRC)
aux_source_directory(${PROJECT_INIT_PATH}/Common/NetWork SUPER_SRC)
aux_source_directory(${PROJECT_INIT_PATH}/Common/Object SUPER_SRC)
aux_source_directory(${PROJECT_INIT_PATH}/Common/Other SUPER_SRC)
aux_source_directory(${PROJECT_INIT_PATH}/Common/Thread SUPER_SRC)
aux_source_directory(${PROJECT_INIT_PATH}/Common/Timer SUPER_SRC)
aux_source_directory(${PROJECT_INIT_PATH}/Common/Util SUPER_SRC)
aux_source_directory(${PROJECT_INIT_PATH}/Common/Script SUPER_SRC)
aux_source_directory(${PROJECT_INIT_PATH}/Common/Service SUPER_SRC)

include_directories(${PROJECT_INIT_PATH}/Protocol)


include_directories(${PROJECT_SOURCE_DIR})
aux_source_directory(${PROJECT_SOURCE_DIR}/Manager SUPER_SRC)

aux_source_directory(${PROJECT_INIT_PATH}/Protocol/Protocol SUPER_SRC)

aux_source_directory(${PROJECT_SOURCE_DIR}/Main SUPER_SRC)
LINK_DIRECTORIES(${PROJECT_INIT_PATH}/Libs/lib)

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_BUILD_TYPE Debug)

if(MSVC)
    
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    file(REMOVE_RECURSE ${PROJECT_SOURCE_DIR}/CMakeFiles)
    file(GLOB_RECURSE ClientFiles ${CMAKE_CURRENT_SOURCE_DIR} "*.h" "*.cpp")
    
    #list(APPEND ServerFiles CommonFiles files)
    foreach(file_name ${CommonFiles})
        source_group_by_dir(${PROJECT_INIT_PATH} ${file_name})
    endforeach()

    foreach(file_name ${ClientFiles})
        source_group_by_dir(${PROJECT_SOURCE_DIR} ${file_name})
    endforeach()

    ADD_EXECUTABLE(Client ${CommonFiles} ${ClientFiles})
    TARGET_LINK_LIBRARIES(Client Lua53 protobuf)
else()
    ADD_EXECUTABLE(Client ${SUPER_SRC})
    TARGET_LINK_LIBRARIES(Client Lua53 protobuf pthread dl)
endif()

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)



