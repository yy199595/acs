#include"LocalLuaService.h"
#include<NetWork/NetLuaAction.h>
namespace SoEasy
{
	LocalLuaService::LocalLuaService(lua_State * luaEnv, int index)
		: mServiceIndex(0), mLuaEnv(nullptr)
	{

		mDefultActionMap["OnInit"] = 0;
		mDefultActionMap["OnComplete"] = 0;

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
					int ref = luaL_ref(luaEnv, LUA_REGISTRYINDEX);
					auto iter = this->mDefultActionMap.find(key);
					if (iter != this->mDefultActionMap.end())
					{
						this->mDefultActionMap[key] = ref;
					}
					else
					{
						NetLuaAction * luaAction = new NetLuaAction(luaEnv, key, ref);
						this->mActionMap.emplace(key, luaAction);
					}				
				}
				else
				{
					lua_pop(this->mLuaEnv, 1);
				}			
			}
		}
	}
	LocalLuaService::~LocalLuaService()
	{

	}

	bool LocalLuaService::OnInit()
	{
		auto iter = this->mDefultActionMap.find("OnInit");
		if (iter != this->mDefultActionMap.end())
		{
			int ref = iter->second;
			this->mDefultActionMap.erase(iter);
			lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, ref);
			if (lua_isfunction(this->mLuaEnv, -1))
			{
				luaL_unref(mLuaEnv, LUA_REGISTRYINDEX, ref);
				if (lua_pcall(this->mLuaEnv, 0, 1, 0) != 0)
				{
					SayNoDebugError(this->GetServiceName() << ":" << lua_tostring(this->mLuaEnv, -1));
					return false;
				}
				if (!lua_toboolean(this->mLuaEnv, -1))
				{
					SayNoDebugError(this->GetServiceName() << " init fail");
					return false;
				}
			}
		}
		return ServiceBase::OnInit();
	}

	bool LocalLuaService::HasMethod(const std::string & name)
	{
		auto iter = this->mActionMap.find(name);
		return iter != this->mActionMap.end();
	}

	bool LocalLuaService::InvokeMethod(const std::string & method, shared_ptr<NetWorkPacket> reqData)
	{
		auto iter = this->mActionMap.find(method);
		if (iter == this->mActionMap.end())
		{
			return false;
		}
		NetLuaAction * luaAction = iter->second;
		XCode code = luaAction->Invoke(reqData);
		long long callbackId = reqData->callback_id();
		if (code != XCode::LuaCoroutineReturn && callbackId != 0)
		{
			reqData->clear_action();
			reqData->clear_service();
			reqData->clear_message_data();
			reqData->set_error_code(code);
			return this->ReplyMessage(callbackId, reqData);
		}
		return true;
	}

	bool LocalLuaService::InvokeMethod(const std::string & address, const std::string & method, SharedPacket reqData)
	{
		auto iter = this->mActionMap.find(method);
		if (iter == this->mActionMap.end())
		{
			return false;
		}
		NetLuaAction * luaAction = iter->second;
		XCode code = luaAction->Invoke(address, reqData);
		if (code != XCode::LuaCoroutineReturn && reqData->callback_id() != 0)
		{
			reqData->clear_action();
			reqData->clear_service();
			reqData->clear_message_data();
			reqData->set_error_code(code);
			return this->ReplyMessage(address, reqData);
		}
		return true;
	}

}