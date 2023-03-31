//
// Created by mac on 2022/5/16.
//

#ifndef SERVER_JSON_H
#define SERVER_JSON_H
#include"Script/Lua/LuaInclude.h"
namespace Lua
{
	namespace RapidJson
	{
		extern int Encode(lua_State * lua);
		extern int Decode(lua_State * lua);
        extern void Write(lua_State * lua, const std::string & json);
        extern void Read(lua_State * lua, int index, std::string * json);
    };
}

#endif //SERVER_JSON_H
