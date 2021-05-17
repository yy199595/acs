#include"RedisLuaTask.h"
#include<Util/NumberHelper.h>
#include<Manager/RedisManager.h>
#include<Coroutine/CoroutineManager.h>
namespace SoEasy
{
	RedisLuaTask::RedisLuaTask(RedisManager * mgr, long long taskId, const std::string & cmd, lua_State * lua, int ref)
		:RedisTaskBase(mgr, taskId, cmd)
	{
		this->mLuaEnv = lua;
		this->mCoroutienRef = ref;
	}

	void RedisLuaTask::OnTaskFinish()
	{
		lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->mCoroutienRef);
		if (lua_isthread(this->mLuaEnv, -1))
		{
			lua_State * coroutine = lua_tothread(this->mLuaEnv, -1);
			if (lua_getfunction(this->mLuaEnv, "JsonUtil", "ToObject"))
			{
				const char * json = this->mQueryJsonData.c_str();
				const size_t size = this->mQueryJsonData.size();
				lua_pushlstring(this->mLuaEnv, json, size);
				if (lua_pcall(this->mLuaEnv, 1, 1, 0) != 0)
				{
					SayNoDebugError("[lua error] " << lua_tostring(mLuaEnv, -1));
				}
			}
			lua_resume(coroutine, this->mLuaEnv, 1);
		}
		if (this->GetErrorCode() != XCode::Successful)
		{
			SayNoDebugError("[redis error ]" << this->GetErrorStr());
		}
	}

	void RedisLuaTask::OnQueryFinish(QuertJsonWritre & jsonWriter)
	{
		SayNoAssertRet_F(jsonWriter.Serialization(mQueryJsonData));
	}

	shared_ptr<RedisLuaTask> RedisLuaTask::Create(lua_State * lua, int index, const char * cmd)
	{
		if (!lua_isthread(lua, index))
		{
			return nullptr;
		}
		Applocation * app = Applocation::Get();
		lua_State * coroutine = lua_tothread(lua, index);
		int ref = luaL_ref(lua, LUA_REGISTRYINDEX);
		RedisManager * redisManager = app->GetManager<RedisManager>();
		if (redisManager == nullptr)
		{
			return nullptr;
		}
		long long taskId = NumberHelper::Create();
		return std::make_shared<RedisLuaTask>(redisManager, taskId, cmd, lua, ref);
	}
}