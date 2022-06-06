//
// Created by zmhy0073 on 2022/1/8.
//

#include"LuaServiceTaskSource.h"
#include"Script/UserDataParameter.h"
namespace Sentry
{
    LuaServiceTaskSource::LuaServiceTaskSource(lua_State * lua)
    {
		this->mRef = 0;
		this->mLua = lua;
        this->mCode = XCode::LuaCoroutineWait;
    }

	LuaServiceTaskSource::~LuaServiceTaskSource()
	{
		if(this->mRef != 0)
		{
			luaL_unref(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		}
	}

    XCode LuaServiceTaskSource::Await()
    {
        if(this->mCode == XCode::LuaCoroutineWait) {
            return this->mTaskSource.Await();
        }
        return this->mCode;
    }

	bool LuaServiceTaskSource::GetRef()
	{
		if(this->mRef != 0)
		{
			lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
			return true;
		}
		return false;
	}

	int LuaServiceTaskSource::SetResult(lua_State* lua)
	{
		std::shared_ptr<LuaServiceTaskSource> luaServiceTaskSource =
			Lua::UserDataParameter::Read<std::shared_ptr<LuaServiceTaskSource>>(lua, 1);
		luaServiceTaskSource->mCode = (XCode)lua_tointeger(lua, 2);
		if(luaServiceTaskSource->mCode == XCode::Successful && !lua_isnil(lua, 3))
		{
			luaServiceTaskSource->mRef = luaL_ref(lua, LUA_REGISTRYINDEX);
		}
		luaServiceTaskSource->mTaskSource.SetResult(luaServiceTaskSource->mCode);
	}
}