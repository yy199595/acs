//
// Created by mac on 2022/5/30.
//

#include"LuaHttp.h"
#include"App/App.h"
#include"Script/UserDataParameter.h"
#include"Async/RpcTask/RpcTaskSource.h"
#include"Network/Http/HttpAsyncRequest.h"
#include"Component/Http/HttpComponent.h"
using namespace Sentry;
namespace Lua
{
	int Http::Get(lua_State* lua)
	{
		lua_pushthread(lua);
		if (!lua_isuserdata(lua, 1))
		{
			luaL_error(lua, "first parameter must httpComponent point");
			return 0;
		}
		if (!lua_isstring(lua, 2))
		{
			luaL_error(lua, "url must string point");
		}
		size_t size = 0;
		TaskComponent* taskComponent = App::Get()->GetTaskComponent();
		std::shared_ptr<LuaRpcTaskSource> luaRpcTaskSource(new LuaRpcTaskSource(lua));
		HttpComponent* httpComponent = UserDataParameter::Read<HttpComponent*>(lua, 1);
		std::shared_ptr<std::string> url(new std::string(lua_tolstring(lua, 2, &size), size));
		taskComponent->Start([luaRpcTaskSource, httpComponent, url, lua]()
		{
			std::string json;
			std::shared_ptr<HttpAsyncResponse> httpResponse = httpComponent->Get(*url);
			if (httpResponse != nullptr && httpResponse->ToJson(json))
			{
				luaRpcTaskSource->SetJson(json);
				return;
			}
			luaRpcTaskSource->SetResult();
		});
		return luaRpcTaskSource->Yield();
	}

	int Http::Post(lua_State* lua)
	{
		lua_pushthread(lua);
		if (!lua_isuserdata(lua, 1))
		{
			luaL_error(lua, "first parameter must httpComponent point");
			return 0;
		}
		if (!lua_isstring(lua, 2))
		{
			luaL_error(lua, "url must string point");
			return 0;
		}
		if (!lua_isstring(lua, 3))
		{
			luaL_error(lua, "content must string");
			return 0;
		}

		size_t size = 0;
		TaskComponent* taskComponent = App::Get()->GetTaskComponent();
		std::shared_ptr<LuaRpcTaskSource> luaRpcTaskSource(new LuaRpcTaskSource(lua));
		HttpComponent* httpComponent = UserDataParameter::Read<HttpComponent*>(lua, 1);
		std::shared_ptr<std::string> url(new std::string(lua_tolstring(lua, 2, &size), size));
		std::shared_ptr<std::string> data(new std::string(lua_tolstring(lua, 3, &size), size));
		taskComponent->Start([luaRpcTaskSource, httpComponent, url, data, lua]()
		{
			std::string json;
			std::shared_ptr<HttpAsyncResponse> httpResponse = httpComponent->Post(*url, *data);
			if (httpResponse != nullptr && httpResponse->ToJson(json))
			{
				luaRpcTaskSource->SetJson(json);
				return;
			}
			luaRpcTaskSource->SetResult();
		});
		return luaRpcTaskSource->Yield();
	}
}