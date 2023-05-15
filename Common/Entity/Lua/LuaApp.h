//
// Created by yjz on 2022/5/15.
//

#ifndef _LUAAPP_H_
#define _LUAAPP_H_
#include<memory>
#include<string>
#include"Lua/Engine/Define.h"

namespace Msg
{
	class Packet;
}
namespace Lua
{
	namespace LuaApp
	{
		extern int Send(lua_State * lua);
		extern int Call(lua_State * lua);
		extern int GetAddr(lua_State * lua);
		extern int GetComponent(lua_State * lua);
		extern int MakeRequest(lua_State * lua, std::shared_ptr<Msg::Packet> & message, std::string & addr, std::string & response);
	};
}

#endif //_LUAAPP_H_
