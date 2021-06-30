#include"LocalLuaService.h"
#include<Pool/ProtocolPool.h>
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

	XCode LocalLuaService::InvokeMethod(const SharedPacket requestData, SharedPacket responseData)
	{	
		const static std::string luaAction = "ServiceProxy.LocalInvoke";
		int ref = this->mScriptManager->GetGlobalReference(luaAction);
		lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, ref);

		if (!lua_isfunction(this->mLuaEnv, -1))
		{
			SayNoDebugError("not find function " << luaAction);
			return XCode::CallLuaFunctionFail;
		}
		lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->mServiceIndex);
		if (!lua_istable(this->mLuaEnv, -1))
		{
			return XCode::CallLuaFunctionFail;
		}
		const std::string & method = requestData->method();
		lua_pushstring(this->mLuaEnv, method.c_str());
		lua_pushinteger(this->mLuaEnv, requestData->entityid());
		lua_pushinteger(this->mLuaEnv, requestData->rpcid());
		if (!requestData->messagedata().empty())
		{
			if (!requestData->protocname().empty())
			{
				ProtocolPool * pool = ProtocolPool::Get();
				Message * message = pool->Create(requestData->protocname());
				if (message != nullptr)
				{
					std::string json;
					ProtocHelper::GetJsonString(message, json);
					lua_pushlstring(this->mLuaEnv, json.c_str(), json.size());
				}
				pool->Destory(message);
			}
			else
			{
				const std::string & data = requestData->messagedata();
				lua_pushlstring(this->mLuaEnv, data.c_str(), data.size());
			}
		}
		if (lua_pcall(this->mLuaEnv, 5, 1, 0) != 0)
		{
			const char * err = lua_tostring(this->mLuaEnv, -1);
			SayNoDebugError("call lua " << this->GetServiceName() << "." << method << "fail " << err);
			return XCode::CallLuaFunctionFail;
		}
		return lua_toboolean(this->mLuaEnv, -1) ? XCode::NotResponseMessage : XCode::CallLuaFunctionFail;
	}

	XCode LocalLuaService::InvokeMethod(const std::string &address, const SharedPacket requestData, SharedPacket responseData)
	{		
		const static std::string luaAction = "ServiceProxy.ProxyInvoke";
		int ref = this->mScriptManager->GetGlobalReference(luaAction);
		lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, ref);

		if (!lua_isfunction(this->mLuaEnv, -1))
		{
			SayNoDebugError("not find function " << luaAction);
			return XCode::CallLuaFunctionFail;
		}
		lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->mServiceIndex);
		if (!lua_istable(this->mLuaEnv, -1))
		{
			return XCode::CallLuaFunctionFail;
		}
		const std::string & method = requestData->method();
		lua_pushstring(this->mLuaEnv, method.c_str());
		lua_pushstring(this->mLuaEnv, address.c_str());
		lua_pushinteger(this->mLuaEnv, requestData->entityid());
		lua_pushinteger(this->mLuaEnv, requestData->rpcid());
		if (!requestData->messagedata().empty())
		{
			if (!requestData->protocname().empty())
			{
				ProtocolPool * pool = ProtocolPool::Get();
				Message * message = pool->Create(requestData->protocname());
				if (message != nullptr)
				{
					std::string json;
					ProtocHelper::GetJsonString(message, json);
					lua_pushlstring(this->mLuaEnv, json.c_str(), json.size());
				}
				pool->Destory(message);
			}
			else
			{
				const std::string & data = requestData->messagedata();
				lua_pushlstring(this->mLuaEnv, data.c_str(), data.size());
			}
		}
		if (lua_pcall(this->mLuaEnv, 6, 1, 0) != 0)
		{
			const char * err = lua_tostring(this->mLuaEnv, -1);
			SayNoDebugError("call lua " << this->GetServiceName() << "." << method << "fail " << err);
			return XCode::CallLuaFunctionFail;
		}
		return lua_toboolean(this->mLuaEnv, -1) ? XCode::NotResponseMessage : XCode::CallLuaFunctionFail;
	}

}