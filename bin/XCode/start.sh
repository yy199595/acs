#!/bin/bash

filepath="./XCode.csv"
file_content=""
lua_string=""
cpp_string=""
csharp_string=""
file_count=0

while IFS=$',' read -r name desc; do

    file_content+="name=$name desc=$desc value=$file_count;"
    if [ $file_count -eq 0 ]; then
        echo $name = $file_count  $desc
        lua_string+="    $name = $file_count, --$desc"
        cpp_string+="    static constexpr int $name = $file_count; // $desc"
        csharp_string+="    public const int $name = $file_count; // $desc"
    else
        echo $name = $file_count  $desc
        lua_string+="    $name = $file_count, --$desc"
        cpp_string+="    static constexpr int $name = $file_count; // $desc"
        csharp_string+="    public const int $name = $file_count; // $desc"
    fi
    ((file_count++))
done < <(tail -n +2 "$filepath")

name=$(tail -n 1 "$filepath" | cut -d ',' -f 1)
desc=$(tail -n 1 "$filepath" | cut -d ',' -f 2)
file_content+="name=$name desc=$desc value=$file_count;"
echo $name = $file_count  $desc
lua_string+="    $name = $file_count, --$desc"
cpp_string+="    static constexpr int $name = $file_count; // $desc"
csharp_string+="    public const int $name = $file_count; // $desc"



lua_string="XCode = {\n$lua_string\n}\n return XCode"
cpp_string="#pragma once\n namespace XCode {\n$cpp_string\n};"
csharp_string="public class XCode {\n$csharp_string\n};"

echo -e "$lua_string" > ../script/Common/XCode.lua
echo -e "$cpp_string" > ../../Gen/XCode/XCode.h
echo -e "$csharp_string" > ./XCode.cs
