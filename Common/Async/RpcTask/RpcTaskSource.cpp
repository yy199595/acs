#include "RpcTaskSource.h"
#include"Component/Rpc/RpcHandlerComponent.h"
#ifdef __DEBUG__
#include"Global/ServiceConfig.h"
#endif
#include"Pool/MessagePool.h"
#include"Script/Extension/Json/values.hpp"
namespace Sentry
{
    void RpcTaskSource::OnResponse(std::shared_ptr<com::Rpc_Response> response)
    {
        this->mTaskSource.SetResult(response);
    }

	std::shared_ptr<com::Rpc_Response> RpcTaskSource::Await()
	{
		return this->mTaskSource.Await();
	}
}

namespace Sentry
{
	LuaRpcTaskSource::LuaRpcTaskSource(lua_State* lua)
		: mLua(lua), mRef(0)
	{
		if(lua_isthread(this->mLua, -1))
		{
			this->mRef = luaL_ref(lua, LUA_REGISTRYINDEX);
		}
	}
	LuaRpcTaskSource::~LuaRpcTaskSource()
	{
		if(this->mRef != 0)
		{
			luaL_unref(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		}
	}

	int LuaRpcTaskSource::Yield()
	{
		if(this->mRef == 0)
		{
			luaL_error(this->mLua, "not lua coroutine context yield failure");
			return 0;
		}
		return lua_yield(this->mLua, 0);
	}

	void LuaRpcTaskSource::SetResult(XCode code, std::shared_ptr<Message> response)
	{
		lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		lua_State* coroutine = lua_tothread(this->mLua, -1);
		lua_pushinteger(this->mLua, (int)code);
		if (code == XCode::Successful && response != nullptr)
		{
			std::string json;
			if (Proto::GetJson(response, json))
			{
				rapidjson::extend::StringStream s(json.c_str(), json.size());
				values::pushDecoded(this->mLua, s);
				lua_presume(coroutine, this->mLua, 2);
				return;
			}
		}
		lua_presume(coroutine, this->mLua, 1);
	}
	void LuaRpcTaskSource::SetResult()
	{
		lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		lua_presume(lua_tothread(this->mLua, -1), this->mLua, 1);
	}
}
