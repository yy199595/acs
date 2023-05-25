//
// Created by leyi on 2023/5/25.
//

#ifndef APP_LUAACTOR_H
#define APP_LUAACTOR_H
struct lua_State;
namespace Tendo
{
	namespace LuaActor
	{
		extern int Send(lua_State * l);
		extern int Call(lua_State * l);
		extern int Random(lua_State * l);
		extern int GetListen(lua_State * l);
		extern int SendToClient(lua_State * l);
	};
}


#endif //APP_LUAACTOR_H
