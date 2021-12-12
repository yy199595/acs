#include"LuaServiceComponent.h"

#include<Core/App.h>
#include<Method/LuaServiceMethod.h>
#include<Scene/RpcConfigComponent.h>
namespace GameKeeper
{
    LuaServiceComponent::LuaServiceComponent()
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

    LuaServiceComponent::~LuaServiceComponent()
    {
        luaL_unref(this->mLuaEnv, LUA_REGISTRYINDEX, this->mIdx);
    }

	bool LuaServiceComponent::InitService(const std::string & name, lua_State * luaEnv)
	{
		this->mServiceName = name;
		lua_getglobal(luaEnv, name.c_str());
		if (!lua_istable(luaEnv, -1))
		{
			return false;
		}
		this->mLuaEnv = luaEnv;
		this->mIdx = luaL_ref(luaEnv, LUA_REGISTRYINDEX);
		auto protocolComponent = App::Get().GetComponent<RpcConfigComponent>();

        return protocolComponent->HasService(this->mServiceName);
	}

    bool LuaServiceComponent::Awake()
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

	bool LuaServiceComponent::LateAwake()
    {
        lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->mIdx);
        lua_getfield(this->mLuaEnv, -1, "Start");
        if (lua_isfunction(this->mLuaEnv, -1))
        {
            return false;
        }
        lua_State *coroutine = lua_newthread(this->mLuaEnv);
        lua_pushvalue(this->mLuaEnv, -2);
        lua_xmove(this->mLuaEnv, coroutine, 1);
        lua_resume(coroutine, this->mLuaEnv, 0);
        return true;
    }

  //  XCode LuaServiceComponent::InvokeMethod(PacketMapper *messageData)
  //  {
  //      const static std::string luaAction = "ServiceProxy.LocalInvoke";
  //      int ref = this->mScriptManager->GetGlobalReference(luaAction);
  //      lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, ref);

  //      if (!lua_isfunction(this->mLuaEnv, -1))
  //      {
  //          LOG_ERROR("not find function " << luaAction);
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
  //          LOG_ERROR("call lua " << this->GetServiceName() << "." << method << "fail " << err);
  //          return XCode::CallLuaFunctionFail;
  //      }*/
		//return XCode::NotResponseMessage;
  //  }
}