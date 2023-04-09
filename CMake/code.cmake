set(code_path ${PROJECT_SOURCE_DIR}/bin/XCode/XCode.csv)

set(cpp_path ${PROJECT_SOURCE_DIR}/Gen/XCode/XCode.h)
set(lua_path ${PROJECT_SOURCE_DIR}/bin/script/Common/XCode.lua)

file(READ ${code_path} file_contents)
string(REGEX MATCHALL "[^\n^]+" file_lines "${file_contents}")

set(counter 0)
list(REMOVE_AT file_lines ${counter}) #移除第一行

file(REMOVE ${cpp_path})
file(REMOVE ${lua_path})
file(WRITE ${cpp_path} "#pragma once\nnamespace XCode \n{\n")
file(WRITE ${lua_path} "XCode = \n{\n")

function(write_cpp name desc index)
    file(APPEND ${cpp_path} "\t constexpr int ")
    file(APPEND ${cpp_path} ${name})
    file(APPEND ${cpp_path} " = ")
    file(APPEND ${cpp_path} ${counter})
    file(APPEND ${cpp_path} ";//")
    file(APPEND ${cpp_path} ${desc})
    file(APPEND ${cpp_path} "\n")

endfunction()

function(write_lua name desc index)
    file(APPEND ${lua_path} "\t")
    file(APPEND ${lua_path} ${name})
    file(APPEND ${lua_path} " = ")
    file(APPEND ${lua_path} ${counter})
    file(APPEND ${lua_path} ",--")
    file(APPEND ${lua_path} ${desc})
    file(APPEND ${lua_path} "\n")
endfunction()

foreach(line ${file_lines})
    string(REGEX MATCHALL "[^,]+" split_list "${line}")
    list(GET split_list 0 name)
    list(GET split_list 1 desc)
    write_cpp(${name} ${desc} ${counter})
    write_lua(${name} ${desc} ${counter})
    math(EXPR counter "${counter} + 1")
endforeach()
file(APPEND ${cpp_path} "};")
file(APPEND ${lua_path} "}")