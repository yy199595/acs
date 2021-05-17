#pragma once
#include"RedisDefine.h"
#include<Script/LuaInclude.h>
#include<Thread/ThreadTaskAction.h>
#define RedisLuaArgvSize 10
namespace SoEasy
{
	class QuertJsonWritre;
	class RedisLuaTask : public ThreadTaskAction
	{
	public:
		RedisLuaTask(RedisManager * mgr, long long taskId, lua_State * lua, int ref);
	public:
		void InitCommand(std::vector<std::string> & command);
		
		long long GetCoroutineId() { return mCoreoutineId; }
	public:
		void OnTaskFinish() final;  //执行完成之后在主线程调用
		void InvokeInThreadPool(long long threadId) final;	//在线程池执行的任务
	public:
		static bool Start(lua_State * lua, int index, std::vector<std::string> & cmmand);
	private:
		void EndWriteJson(QuertJsonWritre & jsonWrite);
	private:		
		std::string mFormat;
		long long mCoreoutineId;
		RedisManager * mRedisManager;
		std::vector<std::string> mCommand;
		const char * mArgvArray[RedisLuaArgvSize];
		size_t mArgvSizeArray[RedisLuaArgvSize];
	private:
		int mCoroutienRef;
		lua_State * mLuaEnv;
	private:
		XCode mErrorCode;
		std::string mErrorString;
		std::string mQueryJsonData;
	};
}