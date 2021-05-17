#include"RedisLuaTask.h"
#include<Util/NumberHelper.h>
#include<Manager/RedisManager.h>
#include<Coroutine/CoroutineManager.h>
namespace SoEasy
{
	RedisLuaTask::RedisLuaTask(RedisManager * mgr, long long taskId, lua_State * lua, int ref)
		:ThreadTaskAction(mgr, taskId)
	{
		this->mLuaEnv = lua;
		this->mRedisManager = mgr;
		this->mCoroutienRef = ref;
	}
	void RedisLuaTask::InitCommand(std::vector<std::string> & command)
	{
		this->mCommand = command;
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
		if (this->mErrorCode != XCode::Successful)
		{
			SayNoDebugError("[redis error ]" << this->mErrorString);
		}
	}

	void RedisLuaTask::InvokeInThreadPool(long long threadId)
	{
		QuertJsonWritre jsonWrite;
		RedisSocket * redisSocket = this->mRedisManager->GetRedisSocket(threadId);
		if (redisSocket == nullptr)
		{
			this->mErrorString = "redis scoket null";
			this->mErrorCode = XCode::RedisSocketIsNull;
			this->EndWriteJson(jsonWrite);
			return;
		}
		

		for (size_t index = 0; index < this->mCommand.size(); index++)
		{
			this->mArgvArray[index] = this->mCommand[index].c_str();
			this->mArgvSizeArray[index] = this->mCommand[index].size();
		}
		const int size = (int)this->mCommand.size();
		redisReply * replay = (redisReply*)redisCommandArgv(redisSocket,size, this->mArgvArray, this->mArgvSizeArray);

		//redisReply * replay = (redisReply*)redisvCommand(redisSocket, this->mFormat.c_str(), this->mCommand);
		if (replay == nullptr)
		{
			this->mErrorString = "redis replay null";
			this->mErrorCode = XCode::RedisReplyIsNull;
			this->EndWriteJson(jsonWrite);
			return;
		}
		switch (replay->type)
		{
		case REDIS_REPLY_STATUS:
			this->mErrorCode = XCode::Successful;
			break;
		case REDIS_REPLY_ERROR:
			this->mErrorCode = RedisInvokeFailure;
			this->mErrorString.assign(replay->str, replay->len);
			break;
		case REDIS_REPLY_INTEGER:
			this->mErrorCode = XCode::Successful;
			jsonWrite.Write("data", replay->integer);
			break;
		case REDIS_REPLY_NIL:
			this->mErrorCode = XCode::Successful;
			jsonWrite.Write("data");
			break;
		case REDIS_REPLY_STRING:
			this->mErrorCode = XCode::Successful;
			jsonWrite.Write("data", replay->str, replay->len);
			break;
		case REDIS_REPLY_ARRAY:
			jsonWrite.StartWriteArray("data");
			this->mErrorCode = XCode::Successful;
			for (size_t index = 0; index < replay->elements; index++)
			{
				redisReply * redisData = replay->element[index];
				if (redisData->type == REDIS_REPLY_INTEGER)
				{
					jsonWrite.Write(redisData->integer);
				}
				else if (redisData->type == REDIS_REPLY_STRING)
				{
					jsonWrite.Write(redisData->str, redisData->len);
				}
				else if (redisData->type == REDIS_REPLY_NIL)
				{
					jsonWrite.Write();
				}
			}
			jsonWrite.EndWriteArray();
			break;
		}
		freeReplyObject(replay);
		this->EndWriteJson(jsonWrite);
	}

	bool RedisLuaTask::Start(lua_State * lua, int index, std::vector<std::string> & cmmand)
	{
		if (!lua_isthread(lua, index))
		{
			return false;
		}
		Applocation * app = Applocation::Get();
		lua_State * coroutine = lua_tothread(lua, index);
		int ref = luaL_ref(lua, LUA_REGISTRYINDEX);
		RedisManager * redisManager = app->GetManager<RedisManager>();
		if (redisManager == nullptr)
		{
			return false;
		}
		long long taskId = NumberHelper::Create();
		shared_ptr<RedisLuaTask> taskAction = std::make_shared<RedisLuaTask>(redisManager, taskId, lua, ref);
		taskAction->InitCommand(cmmand);
		return redisManager->StartTaskAction(taskAction);
	}
	void RedisLuaTask::EndWriteJson(QuertJsonWritre & jsonWrite)
	{
		jsonWrite.Write("code", (long long)this->mErrorCode);
		jsonWrite.Write("error", this->mErrorString.c_str(), this->mErrorString.size());
		if (!jsonWrite.Serialization(mQueryJsonData))
		{
			this->mErrorCode = XCode::RedisJsonParseFail;
			SayNoDebugFatal("mysql result cast json failure");
		}
	}
}