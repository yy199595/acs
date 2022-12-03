//
// Created by yjz on 2022/6/15.
//

#include"LuaRedis.h"
#include"Json/Lua/Json.h"
#include"Client/RedisDefine.h"
#include"Component/ProtoComponent.h"
#include"Component/RedisScriptComponent.h"
#include"Component/RedisComponent.h"

using namespace Sentry;
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
        const char* name = luaL_checkstring(lua, 1);
        const char* command = luaL_checkstring(lua, 2);
        TcpRedisClient * redisClientContext = redisComponent->GetClient(name);
        if (redisClientContext == nullptr)
        {
            luaL_error(lua, "not find redis client %s\n", name);
            return 0;
        }
        std::shared_ptr<RedisRequest> request(new RedisRequest(command));
        int count = (int) luaL_len(lua, 4);
        for (int i = 0; i < count; i++)
        {
            lua_geti(lua, 4, i + 1);
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
                Lua::Json::Read(lua, index, &json);
                request->AddParameter(json);
            }
            lua_pop(lua, 1);
        }

        std::shared_ptr<LuaRedisTask> luaRedisTask = request->MakeLuaTask(lua);
        long long id = luaRedisTask->GetRpcId();
		redisClientContext->Send(request);
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
        RedisScriptComponent* redisScriptComponent = App::Inst()->GetComponent<RedisScriptComponent>();
        if (redisScriptComponent == nullptr)
        {
            luaL_error(lua, "RedisScriptComponent Is Null");
            return 0;
        }
        std::string json;
        lua_pushthread(lua);
        const char *name = luaL_checkstring(lua, 1);
        const char *fullName = luaL_checkstring(lua, 2);
        if (lua_isstring(lua, 3))
        {
            size_t size = 0;
            const char * str = lua_tolstring(lua, 3, &size);
            json.append(str, size);
        }
        else if (lua_istable(lua, 3))
        {
            Lua::Json::Read(lua, 3, &json);
        }
        else
        {
            luaL_error(lua, "parameter muset table or string");
            return 0;
        }
        TcpRedisClient * redisClientContext = redisComponent->GetClient(name);
        if (redisClientContext == nullptr)
        {
            luaL_error(lua, "not find redis client %s\n", name);
            return 0;
        }
        std::shared_ptr<RedisRequest> request = redisScriptComponent->MakeLuaRequest(fullName, json);

		redisClientContext->Send(request);
        std::shared_ptr<LuaRedisTask> luaRedisTask = request->MakeLuaTask(lua);
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
        const char* name = luaL_checkstring(lua, 1);
        const char* command = luaL_checkstring(lua, 2);
        TcpRedisClient* redisClientContext = redisComponent->GetClient(name);
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
        redisClientContext->Send(request);
        return 0;
    }
}