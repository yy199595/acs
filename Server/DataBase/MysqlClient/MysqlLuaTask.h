#pragma once
#include<Script/LuaInclude.h>
#include<Thread/ThreadTaskAction.h>
namespace SoEasy
{
	class MysqlManager;
	class QuertJsonWritre;
	class MysqlLuaTask : public ThreadTaskAction
	{
	public:
		MysqlLuaTask(MysqlManager * mgr, long long taskId, 
			lua_State * lua, int ref, const std::string & db, const std::string & sql);
	public:
		 void OnTaskFinish() final;
		 void InvokeInThreadPool(long long threadId) final;
	public:
		static bool Start(lua_State * lua, int index, const std::string & db, const std::string & sql);
	private:
		void EndWriteJson(QuertJsonWritre & jsonWrite);
	private:
		XCode mErrorCode;
		std::string mErrorStr;
	private:
		int mCroutineRef;
		lua_State * mLuaEnv;
		std::string mCommandSql;
		std::string mDataBaseName;
		std::string mQueryJsonData;
		class MysqlManager * mMysqlManager;
	};
}