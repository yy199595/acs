#pragma once

#include <Script/LuaInclude.h>

namespace LuaProtocExtension
{
    //吧一个lua表序列化成一个protoc对象，返回序列化后的二进制
    extern int Serialization(lua_State *lua);

    extern int CreateByTable(lua_State *lua);

}// namespace LuaProtocExtension
