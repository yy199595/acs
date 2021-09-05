#include"LuaServiceProxy.h"

#include<Core/App.h>
#include<NetWork/LuaServiceMethod.h>
#include<Scene/SceneScriptComponent.h>
#include<Scene/SceneProtocolComponent.h>
namespace Sentry
{
    LuaServiceProxy::LuaServiceProxy()
		: mIdx(0), mLuaEnv(nullptr)
    {
       /* if (lua_istable(luaEnv, index))
        {
            this->mLuaEnv = luaEnv;
            this->mServiceIndex = luaL_ref(luaEnv, LUA_REGISTRYINDEX);
            lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->mServiceIndex);
            int tabindex = lua_gettop(luaEnv);
           
        }*/
    }

    LuaServiceProxy::~LuaServiceProxy()
    {
        luaL_unref(this->mLuaEnv, LUA_REGISTRYINDEX, this->mIdx);
    }

	bool LuaServiceProxy::InitService(const std::string name, lua_State * luaEnv)
	{
		this->mServiceName = name;
		lua_getglobal(luaEnv, name.c_str());
		if (!lua_istable(luaEnv, -1))
		{
			return false;
		}
		this->mLuaEnv = luaEnv;
		this->mIdx = luaL_ref(luaEnv, LUA_REGISTRYINDEX);
		SceneProtocolComponent * protocolComponent = Scene::GetComponent<SceneProtocolComponent>();

		
	}

    bool LuaServiceProxy::Awake()
    {
		lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->mIdx);
		if (!lua_istable(this->mLuaEnv, -1))
		{
			return false;
		}
        if (!lua_istable(this->mLuaEnv, -1))
        {
            return false;
        }
        lua_getfield(this->mLuaEnv, -1, "Awake");
        if (lua_isfunction(this->mLuaEnv, -1))
        {
            lua_pcall(this->mLuaEnv, 0, 0, 0);
            if (!lua_toboolean(this->mLuaEnv, -1))
            {
                return false;
            }
        }
		return true;
    }

  //  XCode LuaServiceProxy::InvokeMethod(PacketMapper *messageData)
  //  {
  //      const static std::string luaAction = "ServiceProxy.LocalInvoke";
  //      int ref = this->mScriptManager->GetGlobalReference(luaAction);
  //      lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, ref);

  //      if (!lua_isfunction(this->mLuaEnv, -1))
  //      {
  //          SayNoDebugError("not find function " << luaAction);
  //          return XCode::CallFunctionNotExist;
  //      }
  //      lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->mServiceIndex);
  //      if (!lua_istable(this->mLuaEnv, -1))
  //      {
		//	return XCode::CallFunctionNotExist;
  //      }
  //      /*const std::string & method = messageData->method();
  //      lua_pushstring(this->mLuaEnv, method.c_str());
  //      lua_pushinteger(this->mLuaEnv, messageData->entityid());
  //      lua_pushinteger(this->mLuaEnv, messageData->rpcid());
  //      if (!messageData->messagedata().empty())
  //      {
  //          if (!messageData->protocname().empty())
  //          {
  //              Message * message = GprotocolPool.Create(messageData->protocname());
  //              if (message != nullptr)
  //              {
  //                  std::string json;
  //                  ProtocHelper::GetJsonString(message, json);
  //                  lua_pushlstring(this->mLuaEnv, json.c_str(), json.size());
  //              }
  //              GprotocolPool.Destory(message);
  //          }
  //          else
  //          {
  //              const std::string & data = messageData->messagedata();
  //              lua_pushlstring(this->mLuaEnv, data.c_str(), data.size());
  //          }
  //      }*/
  //      /*if (lua_pcall(this->mLuaEnv, 5, 1, 0) != 0)
  //      {
  //          const char * err = lua_tostring(this->mLuaEnv, -1);
  //          SayNoDebugError("call lua " << this->GetServiceName() << "." << method << "fail " << err);
  //          return XCode::CallLuaFunctionFail;
  //      }*/
		//return XCode::NotResponseMessage;
  //  }
}