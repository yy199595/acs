#include"MysqlLuaTask.h"
#include<Util/NumberHelper.h>
#include<Manager/MysqlManager.h>
namespace SoEasy
{
	MysqlLuaTask::MysqlLuaTask(MysqlManager * mgr, long long taskId, 
		const std::string & db, const std::string & sql, lua_State * lua, int ref)
		:MysqlTaskBase(mgr, taskId, db), mSqlCommand(sql)
	{
		this->mLuaEnv = lua;
		this->mCroutineRef = ref;
	}

	void MysqlLuaTask::OnTaskFinish()
	{
		lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->mCroutineRef);
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
	}

	void MysqlLuaTask::OnQueryFinish(QuertJsonWritre & jsonWriter)
	{
		MysqlTaskBase::OnQueryFinish(jsonWriter);
		if (this->GetErrorCode() != XCode::Successful)
		{
			SayNoDebugError("[mysql error] " << this->GetErrorStr());
		}
		if (!jsonWriter.Serialization(mQueryJsonData))
		{
			SayNoDebugError("[mysql error] result cast json failure");
		}
	}

	bool MysqlLuaTask::Start(lua_State * lua, int index, const std::string & db, const std::string & sql)
	{
		if (!lua_isthread(lua, index))
		{
			return false;
		}
		Applocation * app = Applocation::Get();
		lua_State * coroutine = lua_tothread(lua, index);
		int ref = luaL_ref(lua, LUA_REGISTRYINDEX);
		MysqlManager * mysqlManager = app->GetManager<MysqlManager>();
		if (mysqlManager == nullptr)
		{
			return false;
		}
		long long taskId = NumberHelper::Create();
		shared_ptr<MysqlLuaTask> taskAction =  std::make_shared<MysqlLuaTask>(mysqlManager, taskId, db, sql, lua, ref);
		return mysqlManager->StartTaskAction(taskAction);
	}
}
