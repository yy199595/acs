//
// Created by yjz on 2022/6/15.
//

#include"LuaRedis.h"
#include"Json/Lua/Json.h"
#include"Client/RedisDefine.h"
#include"Component/ProtoComponent.h"
#include"Component/RedisDataComponent.h"

using namespace Sentry;
namespace Lua
{
	int Redis::Run(lua_State* lua)
	{
		lua_pushthread(lua);
		const char* name = lua_tostring(lua, 2);
		const char* command = lua_tostring(lua, 3);
		RedisDataComponent* redisComponent = UserDataParameter::Read<RedisDataComponent*>(lua, 1);
		std::shared_ptr<TcpRedisClient> redisClientContext = redisComponent->GetClient(name);
		if (redisClientContext == nullptr)
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

		std::shared_ptr<LuaRedisTask> luaRedisTask = request->MakeLuaTask(lua);
		redisComponent->AddTask(luaRedisTask);
		redisClientContext->SendCommand(request);
		return luaRedisTask->Await();
	}

	int Redis::Call(lua_State* lua)
	{
		std::string json;
		lua_pushthread(lua);
		const char* name = lua_tostring(lua, 2);
		const char* fullName = lua_tostring(lua, 3);
		Lua::Json::Read(lua, 4, &json);
		RedisDataComponent* redisComponent = UserDataParameter::Read<RedisDataComponent*>(lua, 1);
		std::shared_ptr<TcpRedisClient> redisClientContext = redisComponent->GetClient(name);
		if (redisClientContext == nullptr)
		{
			luaL_error(lua, "not find redis client %s\n", name);
			return 0;
		}
		std::shared_ptr<RedisRequest> request = redisComponent->MakeLuaRequest(fullName, json);

		std::shared_ptr<LuaRedisTask> luaRedisTask = request->MakeLuaTask(lua);
		redisComponent->AddTask(luaRedisTask);
		redisClientContext->SendCommand(request);
		return luaRedisTask->Await();
	}

    int Redis::Send(lua_State *lua)
    {
        std::string json;
        lua_pushthread(lua);
        const char* name = lua_tostring(lua, 2);
        const char* fullName = lua_tostring(lua, 3);
        Lua::Json::Read(lua, 4, &json);
        RedisDataComponent* redisComponent = UserDataParameter::Read<RedisDataComponent*>(lua, 1);
        std::shared_ptr<TcpRedisClient> redisClientContext = redisComponent->GetClient(name);
        if (redisClientContext == nullptr)
        {
            luaL_error(lua, "not find redis client %s\n", name);
            lua_pushboolean(lua, false);
            return 1;
        }
        lua_pushboolean(lua, true);
        redisClientContext->SendCommand(redisComponent->MakeLuaRequest(fullName, json));
        return 1;
    }
}