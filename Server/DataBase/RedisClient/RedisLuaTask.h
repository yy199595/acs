#pragma once
#include"RedisDefine.h"
#include"RedisTaskBase.h"
#include<Script/LuaInclude.h>

#define RedisLuaArgvSize 10
namespace Sentry
{
	class QuertJsonWritre;
	class RedisLuaTask : public RedisTaskBase
	{
	public:
		RedisLuaTask(RedisManager * mgr, const std::string & cmd, lua_State * lua, int ref);
		~RedisLuaTask();
	protected:
		void OnTaskFinish() final;  //执行完成之后在主线程调用
	public:
		static std::shared_ptr<RedisLuaTask> Create(lua_State * lua, int index, const char * cmd);
	private:
		int mCoroutienRef;
		lua_State * mLuaEnv;
	private:
		std::string mQueryJsonData;
	};
}