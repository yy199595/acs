#include"LocalLuaService.h"
#include<Other/ObjectFactory.h>
#include<Manager/ScriptManager.h>
namespace SoEasy
{
	LocalLuaService::LocalLuaService(lua_State * luaEnv, int index)
		: mServiceIndex(0), mLuaEnv(nullptr)
	{	
		if (lua_istable(luaEnv, index))
		{
			this->mLuaEnv = luaEnv;
			this->mServiceIndex = luaL_ref(luaEnv, LUA_REGISTRYINDEX);
			lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->mServiceIndex);
			int tabindex = lua_gettop(luaEnv);
			lua_pushnil(this->mLuaEnv);
			while (lua_next(this->mLuaEnv, tabindex) != 0)
			{
				const char * key = lua_tostring(luaEnv, -2);
				if (lua_isfunction(luaEnv, -1))
				{
					this->mMethodCacheSet.insert(key);	
				}
				lua_pop(this->mLuaEnv, 1);
			}
		}
	}
	LocalLuaService::~LocalLuaService()
	{
		luaL_unref(this->mLuaEnv, LUA_REGISTRYINDEX, this->mServiceIndex);
	}

	bool LocalLuaService::OnInit()
	{
		this->mScriptManager = this->GetManager<ScriptManager>();
		lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->mServiceIndex);
		if (!lua_istable(this->mLuaEnv, -1))
		{
			return false;
		}
		lua_getfield(this->mLuaEnv, -1, "OnInit");
		if (lua_isfunction(this->mLuaEnv, -1))
		{
			lua_pcall(this->mLuaEnv, 0, 0, 0);
			if (!lua_toboolean(this->mLuaEnv, -1))
			{
				return false;
			}
		}
		return ServiceBase::OnInit();
	}

	bool LocalLuaService::HasMethod(const std::string & name)
	{
		auto iter = this->mMethodCacheSet.find(name);
		return iter != this->mMethodCacheSet.end();
	}

	bool LocalLuaService::InvokeMethod(const std::string & method, shared_ptr<NetWorkPacket> reqData)
	{	
		const static std::string luaAction = "ServiceProxy.LocalInvoke";
		int ref = this->mScriptManager->GetGlobalReference(luaAction);
		lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, ref);

		if (!lua_isfunction(this->mLuaEnv, -1))
		{
			SayNoDebugError("not find function " << luaAction);
			return false;
		}
		lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->mServiceIndex);
		if (!lua_istable(this->mLuaEnv, -1))
		{
			return false;
		}
		lua_pushstring(this->mLuaEnv, method.c_str());
		lua_pushinteger(this->mLuaEnv, reqData->operator_id());
		lua_pushinteger(this->mLuaEnv, reqData->callback_id());
		if (!reqData->message_data().empty())
		{
			if (!reqData->protoc_name().empty())
			{
				Message * message = ObjectFactory::Get()->CreateMessage(reqData->protoc_name());
				if (message != nullptr)
				{
					std::string json;
					ProtocHelper::GetJsonString(message, json);
					lua_pushlstring(this->mLuaEnv, json.c_str(), json.size());
				}
			}
			else
			{
				const std::string & data = reqData->message_data();
				lua_pushlstring(this->mLuaEnv, data.c_str(), data.size());
			}
		}
		if (lua_pcall(this->mLuaEnv, 5, 1, 0) != 0)
		{
			const char * err = lua_tostring(this->mLuaEnv, -1);
			SayNoDebugError("call lua " << this->GetServiceName() << "." << method << "fail " << err);
			return false;
		}
		return lua_toboolean(this->mLuaEnv, -1);
	}

	bool LocalLuaService::InvokeMethod(const std::string & address, const std::string & method, SharedPacket reqData)
	{		
		const static std::string luaAction = "ServiceProxy.ProxyInvoke";
		int ref = this->mScriptManager->GetGlobalReference(luaAction);
		lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, ref);

		if (!lua_isfunction(this->mLuaEnv, -1))
		{
			SayNoDebugError("not find function " << luaAction);
			return false;
		}
		lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->mServiceIndex);
		if (!lua_istable(this->mLuaEnv, -1))
		{
			return false;
		}
		lua_pushstring(this->mLuaEnv, method.c_str());
		lua_pushstring(this->mLuaEnv, address.c_str());
		lua_pushinteger(this->mLuaEnv, reqData->operator_id());
		lua_pushinteger(this->mLuaEnv, reqData->callback_id());
		if (!reqData->message_data().empty())
		{
			if (!reqData->protoc_name().empty())
			{
				Message * message = ObjectFactory::Get()->CreateMessage(reqData->protoc_name());
				if (message != nullptr)
				{
					std::string json;
					ProtocHelper::GetJsonString(message, json);
					lua_pushlstring(this->mLuaEnv, json.c_str(), json.size());
				}
			}
			else
			{
				const std::string & data = reqData->message_data();
				lua_pushlstring(this->mLuaEnv, data.c_str(), data.size());
			}
		}
		if (lua_pcall(this->mLuaEnv, 6, 1, 0) != 0)
		{
			const char * err = lua_tostring(this->mLuaEnv, -1);
			SayNoDebugError("call lua " << this->GetServiceName() << "." << method << "fail " << err);
			return false;
		}
		return lua_toboolean(this->mLuaEnv, -1);
	}

}