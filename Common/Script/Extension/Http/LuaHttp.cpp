//
// Created by mac on 2022/5/30.
//

#include"LuaHttp.h"
#include"App/App.h"
#include"Script/UserDataParameter.h"
#include"Async/Lua/LuaWaitTaskSource.h"
#include"Network/Http/HttpAsyncRequest.h"
#include"Component/Http/HttpComponent.h"
#include"Network/Http/HttpRequestClient.h"
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
		const char * str = lua_tolstring(lua, 2, &size);
		HttpComponent* httpComponent = UserDataParameter::Read<HttpComponent*>(lua, 1);
		std::shared_ptr<HttpGetRequest> getRequest = HttpGetRequest::Create(std::string(str, size));
		if(getRequest == nullptr)
		{
			luaL_error(lua, "parse get url : [%s] failure", str);
			return 0;
		}

		TaskComponent* taskComponent = App::Get()->GetTaskComponent();
		std::shared_ptr<LuaWaitTaskSource> luaRpcTaskSource(new LuaWaitTaskSource(lua));
		taskComponent->Start([luaRpcTaskSource, httpComponent, getRequest]()
		{
			std::string json;
			std::shared_ptr<HttpRequestClient> requestClient = httpComponent->CreateClient();
			std::shared_ptr<HttpAsyncResponse> httpResponse = requestClient->Request(getRequest);
			if (httpResponse != nullptr && httpResponse->ToJson(json))
			{
				luaRpcTaskSource->SetJson(json);
				return;
			}
			luaRpcTaskSource->SetResult();
		});
		return luaRpcTaskSource->Await();
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
		const char * str = lua_tolstring(lua, 2, &size);
		TaskComponent* taskComponent = App::Get()->GetTaskComponent();
		std::shared_ptr<LuaWaitTaskSource> luaWaitTaskSource(new LuaWaitTaskSource(lua));
		HttpComponent* httpComponent = UserDataParameter::Read<HttpComponent*>(lua, 1);
		std::shared_ptr<HttpPostRequest> postRequest = HttpPostRequest::Create(std::string(str, size));
		if(postRequest == nullptr)
		{
			luaL_error(lua, "parse post url : [%s] failure", str);
			return 0;
		}
		postRequest->AddBody(lua_tolstring(lua, 3, &size), size);
		taskComponent->Start([luaWaitTaskSource, httpComponent, postRequest]()
		{
			std::string json;
			std::shared_ptr<HttpRequestClient> requestClient = httpComponent->CreateClient();
			std::shared_ptr<HttpAsyncResponse> httpResponse = requestClient->Request(postRequest);
			if (httpResponse != nullptr && httpResponse->ToJson(json))
			{
				luaWaitTaskSource->SetJson(json);
				return;
			}
			luaWaitTaskSource->SetResult();
		});
		return luaWaitTaskSource->Await();
	}
}