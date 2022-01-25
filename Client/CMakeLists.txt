cmake_minimum_required(VERSION 3.1.3)
PROJECT(Client)


#第三方库包含

include_directories(${PROJECT_COMMON_DIR}/Libs/include/rapidjson)
include_directories(${PROJECT_COMMON_DIR}/Libs/include/protobuf)
include_directories(${PROJECT_COMMON_DIR}/Libs/include/asio)
include_directories(${PROJECT_COMMON_DIR}/Libs/include/lua5.3)
include_directories(${PROJECT_COMMON_DIR}/Libs/include/spdlog)
include_directories(${PROJECT_COMMON_DIR}/Libs/include/mysql)

#公共组件

include_directories(${PROJECT_COMMON_DIR}/Common)
include_directories(${PROJECT_COMMON_DIR}/Common/DB)
include_directories(${PROJECT_COMMON_DIR}/Common/DB/Component)


include_directories(${PROJECT_COMMON_DIR}/Protocol)
include_directories(${PROJECT_COMMON_DIR}/Common/Network)
include_directories(${PROJECT_COMMON_DIR}/Common/Component)

include_directories(${PROJECT_SOURCE_DIR})



file(GLOB_RECURSE MainSource "Main/*.h" "Main/*.cpp" "Main/*.hpp" "Main/*.cc")
file(GLOB_RECURSE ManagerSource "Component/*.h" "Component/*.cpp" "Component/*.hpp" "Component/*.cc")
file(GLOB_RECURSE NetworkSource "Network/*.h" "Network/*.cpp" "Network/*.hpp" "Network/*.cc")

LINK_DIRECTORIES(${PROJECT_COMMON_DIR}/Libs/lib)

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_BUILD_TYPE Debug)

list(APPEND ClientFiles ${MainSource} ${ManagerSource} ${NetworkSource})

if(MSVC)
    
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
   
    foreach(file_name ${CommonFiles})
        source_group_by_dir(${PROJECT_COMMON_DIR} ${file_name})
    endforeach()

    foreach(file_name ${ClientFiles})
        source_group_by_dir(${PROJECT_SOURCE_DIR} ${file_name})
    endforeach()

	  foreach(file_name ${Network})
        source_group_by_dir(${PROJECT_SOURCE_DIR} ${file_name})
    endforeach()

    ADD_EXECUTABLE(Client ${AsmFiles} ${CommonFiles} ${ClientFiles})
    TARGET_LINK_LIBRARIES(Client Lua53 libprotobufd libmysql)
else()
    ADD_EXECUTABLE(Client ${AsmFiles} ${CommonFiles} ${ClientFiles})
    TARGET_LINK_LIBRARIES(Client Lua53 protobuf pthread mysqlclient dl)
endif()

target_compile_definitions(Client PRIVATE __DEBUG__)
target_compile_definitions(Client PRIVATE ASIO_STANDALONE)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)



