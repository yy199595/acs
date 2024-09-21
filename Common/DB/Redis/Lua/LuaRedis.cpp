//
// Created by yjz on 2022/6/15.
//

#include"LuaRedis.h"
#include"Entity/Actor/App.h"
#include"Yyjson/Lua/ljson.h"
#include"Redis/Client/RedisDefine.h"
#include"Proto/Component/ProtoComponent.h"
#include"Redis/Component/RedisComponent.h"
#include "Redis/Component/RedisSubComponent.h"
using namespace acs;
namespace Lua
{
	inline void ReadFromIndex(lua_State * lua, int index, ::redis::Request * request)
	{
		switch (lua_type(lua, index))
		{
			case LUA_TSTRING:
			{
				size_t size = 0;
				const char* str = lua_tolstring(lua, index, &size);
				request->AddString(str, size);
				break;
			}
			case LUA_TNUMBER:
			{
				if (lua_isinteger(lua, index))
				{
					long long num = lua_tointeger(lua, index);
					request->AddParameter(num);
					return;
				}
				double num = lua_tonumber(lua, index);
				request->AddParameter(std::to_string(num));
				break;
			}
			case LUA_TTABLE:
			{
				std::string json;
				if(lua::yyjson::read(lua, index, json))
				{
					request->AddParameter(json);
				}
				break;
			}
			case LUA_TBOOLEAN:
			{
				int val = lua_toboolean(lua, index);
				request->AddParameter(val);
				break;
			}
			default:
				luaL_error(lua, "type error : %s  %d", lua_typename(lua, index), lua_type(lua, index));
				return;
		}
	}

	int redis::Run(lua_State* lua)
    {
		static RedisComponent* redisComponent = nullptr;
		if(redisComponent == nullptr)
		{
			redisComponent = App::Get<RedisComponent>();
			if (redisComponent == nullptr)
			{
				luaL_error(lua, "RedisComponent Is Null");
				return 0;
			}
		}
        lua_pushthread(lua);
        const char* command = luaL_checkstring(lua, 1);
        std::unique_ptr<::redis::Request> request = std::make_unique<::redis::Request>();
		{
			request->SetCommand(command);
			int count = (int)luaL_len(lua, 2);
			for (int i = 0; i < count; i++)
			{
				lua_geti(lua, 2, i + 1);
				int index = lua_absindex(lua, -1);
				ReadFromIndex(lua, index, request.get());
				lua_pop(lua, 1);
			}
		}
		int id = 0;
		redisComponent->Send(std::move(request), id);
        return redisComponent->AddTask(id, new LuaRedisTask(lua, id))->Await();
    }

	int redis::Call(lua_State* lua)
    {
		static RedisComponent* redisComponent = nullptr;
		if(redisComponent == nullptr)
		{
			redisComponent = App::Get<RedisComponent>();
			if (redisComponent == nullptr)
			{
				luaL_error(lua, "RedisComponent Is Null");
				return 0;
			}
		}
		RedisLuaData redisLuaData;
		redisLuaData.name = luaL_checkstring(lua, 1);
		switch(lua_type(lua, 2))
		{
			case LUA_TSTRING:
			{
				size_t size = 0;
				const char * str = lua_tolstring(lua, 2, &size);
				redisLuaData.json.append(str, size);
			}
				break;
			case LUA_TTABLE:
				lua::yyjson::read(lua, 2, redisLuaData.json);
				break;
			default:
				luaL_error(lua, "parameter muset table or string");
				return 0;
		}
		int id = 0;
        lua_pushthread(lua);
		redisComponent->Send(redisLuaData, id);
        return redisComponent->AddTask(id, new LuaRedisTask(lua, id))->Await();
    }

    int redis::Send(lua_State *lua)
    {
		static RedisComponent* redisComponent = nullptr;
		if(redisComponent == nullptr)
		{
			redisComponent = App::Get<RedisComponent>();
			if (redisComponent == nullptr)
			{
				luaL_error(lua, "RedisComponent Is Null");
				return 0;
			}
		}
        const char* command = luaL_checkstring(lua, 1);
		std::unique_ptr<::redis::Request> request = std::make_unique<::redis::Request>();
		{
			request->SetCommand(command);
			int count = (int)luaL_len(lua, 2);
			for (int i = 0; i < count; i++)
			{
				lua_geti(lua, 2, i + 1);
				int index = lua_absindex(lua, -1);
				ReadFromIndex(lua, index, request.get());
				lua_pop(lua, 1);
			}
		}
		lua_pushboolean(lua, true);
		redisComponent->Send(std::move(request));
        return 1;
    }

    int redis::SyncRun(lua_State* lua)
    {
		static RedisComponent* redisComponent = nullptr;
		if(redisComponent == nullptr)
		{
			redisComponent = App::Get<RedisComponent>();
			if (redisComponent == nullptr)
			{
				luaL_error(lua, "RedisComponent Is Null");
				return 0;
			}
		}
        const char* command = luaL_checkstring(lua, 1);
        std::unique_ptr<::redis::Request> request = std::make_unique<::redis::Request>();
		{
			request->SetCommand(command);
			int count = (int)luaL_len(lua, 2);
			for (int i = 0; i < count; i++)
			{
				lua_geti(lua, 2, i + 1);
				int index = lua_absindex(lua, -1);
				ReadFromIndex(lua, index, request.get());
				lua_pop(lua, 1);
			}
		}
        std::unique_ptr<::redis::Response> response = redisComponent->SyncRun(std::move(request));
        return response != nullptr ? response->WriteToLua(lua) : 0;
    }

	int redis::Sub(lua_State* lua)
	{
		static RedisSubComponent * subComponent = nullptr;
		if(subComponent == nullptr)
		{
			subComponent = App::Get<RedisSubComponent>();
			if(subComponent == nullptr)
			{
				luaL_error(lua, "not add RedisSubComponent");
				return 0;
			}
		}
		const char* channel = luaL_checkstring(lua, 1);
		if(!subComponent->SubChannel(channel))
		{
			lua_pushboolean(lua, false);
			return 1;
		}
		lua_pushboolean(lua, true);
		return 1;
	}

	int redis::UnSub(lua_State* lua)
	{
		return 0;
	}
}