//
// Created by leyi on 2023/11/8.
//

#ifndef APP_LUACLIENT_H
#define APP_LUACLIENT_H
struct lua_State;
namespace acs
{
	namespace LuaTcpClient
	{
		extern int Send(lua_State * l);
		extern int Call(lua_State * l);
		extern int Close(lua_State * l);
		extern int Connect(lua_State * l);
	}
}

namespace acs
{
	namespace LuaWsClient
	{
		extern int Send(lua_State * l);
		extern int Call(lua_State * l);
		extern int Close(lua_State * l);
		extern int Connect(lua_State * l);
	}
}



#endif //APP_LUACLIENT_H
