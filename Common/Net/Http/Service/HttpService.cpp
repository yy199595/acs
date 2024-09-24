//
// Created by yjz on 2022/1/23.
//
#include"HttpService.h"
#include"Yyjson/Lua/ljson.h"
#include"Lua/Module/LuaModule.h"
#include"Lua/Component/LuaComponent.h"
#include"Rpc/Lua/LuaServiceTaskSource.h"

namespace acs
{

    HttpService::HttpService()
		: mServiceRegister(this)
    {
		this->mLuaModule = nullptr;
    }

    bool HttpService::LateAwake()
    {
		const std::string & name = this->GetName();
		LOG_CHECK_RET_FALSE(HttpConfig::Inst()->HasService(name))
		LuaComponent * luaComponent = this->GetComponent<LuaComponent>();
		if(luaComponent != nullptr)
		{
			this->mLuaModule = luaComponent->LoadModule(name);
		}
		if(!this->OnInit())
		{
			LOG_ERROR("[{}] Init Fail", name)
			return false;
		}
		if(this->mLuaModule != nullptr)
		{
			int code = this->mLuaModule->Call("Awake");
			return code != XCode::CallLuaFunctionFail;
		}
		return true;
    }

	void HttpService::Start()
	{
		this->OnStart();
		IF_NOT_NULL_CALL(this->mLuaModule, Await, "OnStart");
	}

	void HttpService::Complete()
	{
		this->OnComplete();
		IF_NOT_NULL_CALL(this->mLuaModule, Await, "OnComplete")
	}

	void HttpService::OnDestroy()
	{
		this->OnStop();
		if (this->mLuaModule != nullptr)
		{
			this->mLuaModule->Await("OnStop");
			this->mLuaModule = nullptr;
		}
	}

	int HttpService::Invoke(const HttpMethodConfig * config, const http::Request& request, http::Response& response)
	{
		const std::string & method = config->Method;		
		if(this->mLuaModule != nullptr && this->mLuaModule->HasFunction(method))
		{
			if(!config->IsAsync)
			{
				return this->CallLua(method, request, response);
			}
			return this->AwaitCallLua(method, request, response);
		}
		HttpServiceMethod * target = this->mServiceRegister.GetMethod(method);
		return target == nullptr ? XCode::CallFunctionNotExist : target->Invoke(request, response);
	}

	int HttpService::CallLua(const std::string & method, const http::Request& request, http::Response& response)
	{
		this->mLuaModule->GetMetaFunction("__Invoke");
		lua_State * lua = this->mLuaModule->GetLuaEnv();
		{
			lua_pushstring(lua, method.c_str());
			if(request.WriteToLua(lua) == -1)
			{
				return XCode::CallArgsError;
			}
			if(lua_pcall(lua, 3, 2, 0) != LUA_OK)
			{
				const char * err = lua_tostring(lua, -1);
				{
					LOG_ERROR("{} error:{}", method, err);
				}
				return XCode::CallLuaFunctionFail;
			}
			int code = (int)luaL_checkinteger(lua, -2);
			if(code != XCode::Ok)
			{
				return code;
			}
			if (lua_istable(lua, -1))
			{
				json::w::Document document;
				lua::JsonValue jsonValue;
				lua::yyjson::read(lua, -1, jsonValue);
				{
					document.Add("code", code);
					document.Add("data", jsonValue.val);
				}
				response.Json(document);
				return XCode::Ok;
			}
			return XCode::Failure;
		}
	}

	int HttpService::AwaitCallLua(const std::string & method, const http::Request& request, http::Response& response)
	{
		if(!this->mLuaModule->GetMetaFunction("__Call"))
		{
			return XCode::CallServiceNotFound;
		}
		lua_State * lua = this->mLuaModule->GetLuaEnv();
		{
			lua_pushstring(lua, method.c_str());

			request.WriteToLua(lua);
			std::unique_ptr<LuaServiceTaskSource> luaTaskSource =
					std::make_unique<LuaServiceTaskSource>(&response);
			Lua::UserDataParameter::Write(lua, luaTaskSource.get());
			if (lua_pcall(lua, 4, 1, 0) != LUA_OK)
			{
				std::string error;
				this->mLuaModule->SpliteError(error);
				LOG_ERROR("{} error:{}", method, error);
				return XCode::CallLuaFunctionFail;
			}
			return luaTaskSource->Await();
		}
	}
}