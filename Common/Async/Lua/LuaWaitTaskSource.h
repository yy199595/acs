//
// Created by mac on 2022/5/31.
//

#ifndef SERVER_LUAWAITTASKSOURCE_H
#define SERVER_LUAWAITTASKSOURCE_H
#include"Script/LuaInclude.h"
#include"Async/RpcTask/RpcTaskSource.h"
namespace Sentry
{
	// 在lua中等待c++协程
	class LuaWaitTaskSource
	{
	public:
		LuaWaitTaskSource(lua_State* lua);

		~LuaWaitTaskSource();

	public:
		int Await();

		void SetResult();

		template<typename T>
		void SetResult(T result);

		void SetJson(const std::string& json);

		void SetResult(XCode code, std::shared_ptr <Message> response);

	private:
		int mRef;
		lua_State* mLua;
	};

	template<typename T>
	void LuaWaitTaskSource::SetResult(T result)
	{
		lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		lua_State* coroutine = lua_tothread(this->mLua, -1);
		Lua::Parameter::Write(this->mLua, result);
		lua_presume(coroutine, this->mLua, 1);
	}
}
#endif //SERVER_LUAWAITTASKSOURCE_H
