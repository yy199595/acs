local filepath = "./XCode.csv"

local fs = io.open(filepath, "r")

fs:read()

local split = function(s, delim)
    local split = {}
    local pattern = "[^" .. delim .. "]+"
    string.gsub(
        s,
        pattern,
        function(v)
            table.insert(split, v)
        end
    )
    return split
end

local lua_string = ""
local cpp_string = ""
local csharp_string = ""
local file_count = 0
local file_content = {}
while true do
    local line = fs:read()
    if line == nil then
        break
    end
    local content = {}
    local linedata = split(line, "\t")
    content.name = linedata[1]
    content.desc = linedata[2]
    content.value = file_count
    file_count = file_count + 1
    table.insert(file_content, content)
end
for index, conetnt in ipairs(file_content) do
    if index == file_count then
        lua_string = string.format("%s\n\t%s = %s--%s", lua_string, conetnt.name, conetnt.value, conetnt.desc)
        cpp_string = string.format("%s\n\t%s = %s//%s", cpp_string, conetnt.name, conetnt.value, conetnt.desc)
        csharp_string = string.format("%s\n\t%s = %s//%s", csharp_string, conetnt.name, conetnt.value, conetnt.desc)
    else
        lua_string = string.format("%s\n\t%s = %s,--%s", lua_string, conetnt.name, conetnt.value, conetnt.desc)
        cpp_string = string.format("%s\n\t%s = %s,//%s", cpp_string, conetnt.name, conetnt.value, conetnt.desc)
        csharp_string = string.format("%s\n\t%s = %s,//%s", csharp_string, conetnt.name, conetnt.value, conetnt.desc)
    end
end

local lua_write = io.output("../Script/Common/XCode.lua")
local cpp_write = io.output("../../ServerData/XCode/XCode.h")
--local csharp_write = io.output("./XCode.cs")

lua_write:write(string.format("%s%s%s", "XCode =\n{", lua_string, "\n}\nreturn XCode"))
--csharp_write:write(string.format("%s%s%s", "public enum XCode\n{", cpp_string, "\n};"))
cpp_write:write(string.format("%s%s%s", "#pragma once\nenum XCode\n{", csharp_string, "\n};"))
lua_write:close()
cpp_write:close()
--csharp_write:close()
