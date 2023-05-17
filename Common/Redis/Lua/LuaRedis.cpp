//
// Created by yjz on 2022/6/15.
//

#include"LuaRedis.h"
#include"Entity/Actor/App.h"
#include"Util/Json/Lua/Json.h"
#include"Redis/Client/RedisDefine.h"
#include"Proto/Component/ProtoComponent.h"
#include"Redis/Component/RedisComponent.h"
#include"Redis/Component/RedisLuaComponent.h"

using namespace Tendo;
namespace Lua
{
	int Redis::Run(lua_State* lua)
    {
        lua_pushthread(lua);     
        RedisComponent* redisComponent = App::Inst()->GetComponent<RedisComponent>();
        if (redisComponent == nullptr)
        {
            luaL_error(lua, "RedisComponent Is Null");
            return 0;
        }
        const char* command = luaL_checkstring(lua, 1);
        std::shared_ptr<RedisRequest> request(new RedisRequest(command));
        int count = (int) luaL_len(lua, 2);
        for (int i = 0; i < count; i++)
        {
            lua_geti(lua, 2, i + 1);
            int index = lua_absindex(lua, -1);
            if (lua_isstring(lua, index))
            {
                size_t size = 0;
                const char *str = lua_tolstring(lua, index, &size);
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
                Lua::RapidJson::Read(lua, index, &json);
                request->AddParameter(json);
            }
            lua_pop(lua, 1);
        }
		int id = 0;
		redisComponent->Send(request, id);
        std::shared_ptr<LuaRedisTask> luaRedisTask = request->MakeLuaTask(lua, id);
        return redisComponent->AddTask(id, luaRedisTask)->Await();
    }

	int Redis::Call(lua_State* lua)
    {
        RedisComponent* redisComponent = App::Inst()->GetComponent<RedisComponent>();
        if (redisComponent == nullptr)
        {
            luaL_error(lua, "RedisComponent Is Null");
            return 0;
        }
        RedisLuaComponent* redisScriptComponent = App::Inst()->GetComponent<RedisLuaComponent>();
        if (redisScriptComponent == nullptr)
        {
            luaL_error(lua, "RedisScriptComponent Is Null");
            return 0;
        }
        std::string json;
        const char *fullName = luaL_checkstring(lua, 1);
        if (lua_isstring(lua, 2))
        {
            size_t size = 0;
            const char * str = lua_tolstring(lua, 2, &size);
            json.append(str, size);
        }
        else if (lua_istable(lua, 2))
        {
            Lua::RapidJson::Read(lua, 2, &json);
        }
        else
        {
            luaL_error(lua, "parameter muset table or string");
            return 0;
        }
        std::shared_ptr<RedisRequest> request =
            redisScriptComponent->MakeLuaRequest(fullName, json);
        if (lua_isboolean(lua, 3) && !lua_toboolean(lua, 3))
        {
            std::shared_ptr<RedisResponse> response 
                = redisComponent->SyncRun(request);
            return response != nullptr ? response->WriteToLua(lua) : 0;
        }

		int id = 0;
        lua_pushthread(lua);
		if(!redisComponent->Send(request, id))
		{
			luaL_error(lua, "send redis cmd error");
			return 0;
		}
        std::shared_ptr<LuaRedisTask> luaRedisTask = request->MakeLuaTask(lua, id);
        return redisComponent->AddTask(luaRedisTask->GetRpcId(), luaRedisTask)->Await();
    }

    int Redis::Send(lua_State *lua)
    {
        RedisComponent* redisComponent = App::Inst()->GetComponent<RedisComponent>();
        if (redisComponent == nullptr)
        {
            luaL_error(lua, "RedisComponent Is Null");
            return 0;
        }
        const char* command = luaL_checkstring(lua, 1);
        std::shared_ptr<RedisRequest> request =
			std::make_shared<RedisRequest>(command);
        int count = (int)luaL_len(lua, 2);
        for (int i = 0; i < count; i++)
        {
            lua_geti(lua, 2, i + 1);
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
                Lua::RapidJson::Read(lua, index, &json);
                request->AddParameter(json);
            }
            lua_pop(lua, 1);
        }
		if(!redisComponent->Send(request))
		{
			luaL_error(lua, "send redis cmd error");
		}
        return 0;
    }

    int Redis::SyncRun(lua_State* lua)
    {
        RedisComponent* redisComponent = App::Inst()->GetComponent<RedisComponent>();
        if (redisComponent == nullptr)
        {
            luaL_error(lua, "RedisComponent Is Null");
            return 0;
        }
        const char* command = luaL_checkstring(lua, 1);
        std::shared_ptr<RedisRequest> request(new RedisRequest(command));
        int count = (int)luaL_len(lua, 2);
        for (int i = 0; i < count; i++)
        {
            lua_geti(lua, 2, i + 1);
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
                Lua::RapidJson::Read(lua, index, &json);
                request->AddParameter(json);
            }
            lua_pop(lua, 1);
        }
        std::shared_ptr<RedisResponse> response = redisComponent->SyncRun(request);
        return response != nullptr ? response->WriteToLua(lua) : 0;
    }
}