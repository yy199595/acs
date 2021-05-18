#pragma once
#include"MysqlTaskBase.h"
#include<Script/LuaInclude.h>
namespace SoEasy
{
	class MysqlManager;
	class QuertJsonWritre;
	class MysqlLuaTask : public MysqlTaskBase
	{
	public:
		MysqlLuaTask(MysqlManager * mgr, long long taskId, const std::string & db, const std::string & sql, lua_State * lua, int ref);
	protected:
		 void OnTaskFinish() final;
		 void OnQueryFinish(QuertJsonWritre & jsonWriter) override; //查询完成之后调用
	public:
		static bool Start(lua_State * lua, int index, const std::string & db, const std::string & sql);
	private:
		int mCroutineRef;
		lua_State * mLuaEnv;
		std::string mQueryJsonData;
	};
}