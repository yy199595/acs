//
// Created by yjz on 2022/6/15.
//

#include"LuaRedis.h"
#include"DB/Redis/RedisDefine.h"
#include"Script/Extension/Json/Json.h"
#include"Component/Scene/MessageComponent.h"
#include"Component/Redis/MainRedisComponent.h"

using namespace Sentry;
namespace Lua
{
	int Redis::Run(lua_State* lua)
	{
		lua_pushthread(lua);
		const char * name = lua_tostring(lua, 2);
		const char * command = lua_tostring(lua, 3);
		MainRedisComponent* redisComponent = UserDataParameter::Read<MainRedisComponent*>(lua, 1);
		std::shared_ptr<RedisClientContext> redisClientContext = redisComponent->GetClient(name);
		if(redisClientContext == nullptr)
		{
			luaL_error(lua, "not find redis client %s\n", name);
			return 0;
		}
		std::shared_ptr<RedisRequest> request(new RedisRequest(command));
		int count = (int)luaL_len(lua, 4);
		for (int i = 0; i < count; i++)
		{
			lua_geti(lua, 4, i + 1);
			int index = lua_absindex(lua, -1);
			if (lua_isstring(lua, index))
			{
				size_t size = 0;
				const char* str = lua_tolstring(lua, index, &size);
				request->AddString(str, size);
			}
			else if (lua_isinteger(lua, index))
			{
				long long num = lua_tointeger(lua, index);
				request->AddParameter(num);
			}
			else if (lua_istable(lua, index))
			{
				std::string json;
				Lua::Json::Read(lua, index, &json);
				request->AddParameter(json);
			}
			lua_pop(lua, 1);
		}
		return Redis::Send(lua, redisComponent, redisClientContext, request);
	}

	int Redis::Send(lua_State* lua, MainRedisComponent* redisComponent,
		std::shared_ptr<RedisClientContext> redisClientContext, std::shared_ptr<RedisRequest> request)
	{
		std::shared_ptr<LuaRedisTask> luaRedisTask = request->MakeLuaTask(lua);
		if(!redisClientContext->IsOpen())
		{
			TaskComponent * taskComponent = App::Get()->GetTaskComponent();
			taskComponent->Start([request, redisClientContext, redisComponent, luaRedisTask](){
				if(!redisComponent->TryAsyncConnect(redisClientContext))
				{
					luaRedisTask->OnResponse(nullptr);
					return;
				}
				redisComponent->AddRedisTask(luaRedisTask);
				redisClientContext->SendCommand(request);
			});
		}
		else
		{
			redisComponent->AddRedisTask(luaRedisTask);
			redisClientContext->SendCommand(request);
		}
		return luaRedisTask->Await();
	}

	int Redis::Call(lua_State* lua)
	{
		std::string json;
		lua_pushthread(lua);
		const char* name = lua_tostring(lua, 2);
		const char* fullName = lua_tostring(lua, 3);
		Lua::Json::Read(lua, 4, &json);
		MainRedisComponent* redisComponent = UserDataParameter::Read<MainRedisComponent*>(lua, 1);
		std::shared_ptr<RedisClientContext> redisClientContext = redisComponent->GetClient(name);
		if (redisClientContext == nullptr)
		{
			luaL_error(lua, "not find redis client %s\n", name);
			return 0;
		}
		std::shared_ptr<RedisRequest> request =
			redisComponent->MakeLuaRequest(fullName, json);
		return Redis::Send(lua, redisComponent, redisClientContext, request);
	}
}