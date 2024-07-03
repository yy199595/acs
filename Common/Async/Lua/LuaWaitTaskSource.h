//
// Created by mac on 2022/5/31.
//

#ifndef APP_LUAWAITTASKSOURCE_H
#define APP_LUAWAITTASKSOURCE_H

#include"Lua/Engine/LuaParameter.h"
#include"Async/Lua/LuaCoroutine.h"
#include"Rpc/Client/Message.h"
namespace joke
{
	// 在lua中等待c++协程
	class LuaWaitTaskSource
	{
	public:
		explicit LuaWaitTaskSource(lua_State* lua);

		~LuaWaitTaskSource();

	public:
		int Await();

		void SetResult();

		template<typename T>
		void SetResult(T result);
		void SetResult(int code, rpc::Packet * response);
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
        Lua::Coroutine::Resume(coroutine, this->mLua, 1);
	}
}
#endif //APP_LUAWAITTASKSOURCE_H
