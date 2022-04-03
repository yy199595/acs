//
// Created by mac on 2022/3/31.
//

#ifndef SERVER_LUATASKSOURCE_H
#define SERVER_LUATASKSOURCE_H
#include"Async/TaskSource.h"
#include"Script/LuaInclude.h"
#include"Script/LuaParameter.h"
namespace Sentry
{
	class LuaTaskSource : public TaskSourceBase
	{
	 public:
		explicit LuaTaskSource();
		~LuaTaskSource();
	public:
		template<typename T> T Await();
		static int SetResult(lua_State * lua);
	private:
		int ref;
		lua_State * mLua;
	};

	template<typename T>
	T LuaTaskSource::Await()
	{
		this->YieldTask();
		lua_rawgeti(mLua, LUA_REGISTRYINDEX, ref);
		return Lua::Parameter::Read<T>(this->mLua, -1);
	}
}
#endif //SERVER_LUATASKSOURCE_H
