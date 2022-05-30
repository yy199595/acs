//
// Created by mac on 2022/5/30.
//

#include"LuaHttp.h"
#include"App/App.h"
#include"Script/UserDataParameter.h"
#include"Component/Http/HttpComponent.h"
using namespace Sentry;
namespace Lua
{
	int Http::Get(lua_State* lua)
	{
		lua_pushthread(lua);
		if(!lua_isuserdata(lua, 1))
		{
			luaL_error(lua, "first parameter must httpComponent point");
			return 0;
		}
		if(!lua_isstring(lua, 2))
		{
			luaL_error(lua, "url must string point");
		}
		size_t size = 0;
		App::Get()->
		HttpComponent * httpComponent = UserDataParameter::Read<HttpComponent*>(lua, 1);
		const char * content = lua_tolstring(lua, 2, &size);
		httpComponent->Get(std::string(content, size));
	}

	int Http::Post(lua_State* lua)
	{

	}
}