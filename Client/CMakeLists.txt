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
include_directories(${PROJECT_INIT_PATH}/Protocol)
include_directories(${PROJECT_SOURCE_DIR}/Manager)

include_directories(${PROJECT_SOURCE_DIR})

file(GLOB_RECURSE MainSource "Main/*.h" "Main/*.cpp" "Main/*.hpp" "Main/*.cc")
file(GLOB_RECURSE ManagerSource "Manager/*.h" "Manager/*.cpp" "Manager/*.hpp" "Manager/*.cc")

LINK_DIRECTORIES(${PROJECT_INIT_PATH}/Libs/lib)

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_BUILD_TYPE Debug)

list(APPEND ClientFiles ${MainSource} ${ManagerSource})

if(MSVC)
    
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
   
    foreach(file_name ${CommonFiles})
        source_group_by_dir(${PROJECT_INIT_PATH} ${file_name})
    endforeach()

    foreach(file_name ${ClientFiles})
        source_group_by_dir(${PROJECT_SOURCE_DIR} ${file_name})
    endforeach()

    ADD_EXECUTABLE(Client ${CommonFiles} ${ClientFiles})
    TARGET_LINK_LIBRARIES(Client Lua53 libprotobufd)
else()
    ADD_EXECUTABLE(Client ${CommonFiles} ${ClientFiles})
    TARGET_LINK_LIBRARIES(Client Lua53 protobuf pthread dl)
endif()

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)



