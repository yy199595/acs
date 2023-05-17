//
// Created by yjz on 2022/1/23.
//
#include"HttpService.h"
#include"Lua/Module/LuaModule.h"
#include"Util/Json/Lua/Json.h"
#include"Lua/Component/LuaScriptComponent.h"
#include "Rpc/Lua/LuaServiceTaskSource.h"

namespace Tendo
{

    HttpService::HttpService()
		: mServiceRegister(this), mConfig(nullptr)
    {

    }

    bool HttpService::LateAwake()
    {
		const std::string & name = this->GetName();
		this->mConfig = HttpConfig::Inst()->GetConfig(name);
		LuaScriptComponent * luaComponent = this->GetComponent<LuaScriptComponent>();
		if(luaComponent != nullptr)
		{
			this->mLuaModule = luaComponent->LoadModule(name);
		}
		LOG_CHECK_RET_FALSE(this->mConfig && this->OnInit());
		if(this->mLuaModule != nullptr)
		{
			std::vector<const HttpMethodConfig*> methodConfigs;
			this->mConfig->GetMethodConfigs(methodConfigs);
			for (const HttpMethodConfig* methodConfig: methodConfigs)
			{
				this->mLuaModule->GetFunction(methodConfig->Method);
			}
		}
		return true;
    }

	void HttpService::Start()
	{
		this->OnStart();
		if(this->mLuaModule != nullptr)
		{
			this->mLuaModule->Await("OnStart");
		}
	}

	void HttpService::OnSecondUpdate(int tick)
	{
		this->OnSecond(tick);
		if(this->mLuaModule != nullptr)
		{
			this->mLuaModule->Update(tick);
		}
	}

	void HttpService::OnHotFix()
	{
		if(this->mLuaModule != nullptr)
		{
			this->mLuaModule->Hotfix();
		}
	}

	void HttpService::Complete()
	{
		this->OnComplete();
		if(this->mLuaModule != nullptr)
		{
			this->mLuaModule->Await("OnComplete");
		}
	}

	void HttpService::OnDestroy()
	{
		this->OnStop();
		if(this->mLuaModule != nullptr)
		{
			this->mLuaModule->Await("OnStop");
			delete this->mLuaModule;
			this->mLuaModule = nullptr;
		}
	}

	int HttpService::Invoke(const string& method, const std::shared_ptr<Http::Request>& request,
			std::shared_ptr<Http::DataResponse>& response)
	{
		if(this->mLuaModule != nullptr && this->mLuaModule->HasFunction(method))
		{
			const HttpMethodConfig * methodConfig = this->mConfig->GetMethodConfig(method);
			if (methodConfig == nullptr)
			{
				return XCode::NotFoundRpcConfig;
			}
			if(!methodConfig->IsAsync)
			{
				return this->CallLua(method, *request, *response);
			}
			return this->AwaitCallLua(method, *request, *response);
		}
		HttpServiceMethod * target = this->mServiceRegister.GetMethod(method);
		return target == nullptr ? XCode::CallFunctionNotExist : target->Invoke(*request, *response);
	}

	int HttpService::CallLua(const std::string & method, const Http::Request& request, Http::DataResponse& response)
	{
		this->mLuaModule->GetFunction(method);
		lua_State * lua = this->mLuaModule->GetLuaEnv();
		{
			request.WriteToLua(lua);
			if (lua_pcall(lua, 1, 2, 0) != LUA_OK)
			{
				Json::Writer document;
				document.Add("error").Add(lua_tostring(lua, -1));
				document.Add("code").Add((int)XCode::CallLuaFunctionFail);
				response.Json(HttpStatus::OK, document);
				return XCode::CallLuaFunctionFail;
			}
			if (lua_isstring(lua, -1))
			{
				size_t size = 0;
				const char *json = lua_tolstring(lua, -1, &size);
				response.Json(HttpStatus::OK, json, size);
				return XCode::Successful;
			}
			else if (lua_istable(lua, -1))
			{
				std::string data;
				Lua::RapidJson::Read(lua, -1, &data);
				response.Json(HttpStatus::OK, data.c_str(), data.size());
				return XCode::Successful;
			}
		}
		return (int)luaL_checkinteger(lua, -2);
	}

	int HttpService::AwaitCallLua(const std::string & method, const Http::Request& request, Http::DataResponse& response)
	{
		lua_State * lua = this->mLuaModule->GetLuaEnv();
		if(!Lua::Function::Get(lua, "coroutine", "http"))
		{
			return XCode::CallLuaFunctionFail;
		}
		this->mLuaModule->GetFunction(method);
		{
			request.WriteToLua(lua);
		}
		std::unique_ptr<LuaServiceTaskSource> luaTaskSource =
				std::make_unique<LuaServiceTaskSource>(&response);
		Lua::UserDataParameter::Write(lua, luaTaskSource.get());
		if (lua_pcall(lua, 3, 1, 0) != LUA_OK)
		{
			Json::Writer document;
			document.Add("error").Add(lua_tostring(lua, -1));
			document.Add("code").Add((int)XCode::CallLuaFunctionFail);
			response.Json(HttpStatus::OK, document);
			return XCode::CallLuaFunctionFail;
		}
		return luaTaskSource->Await();
	}
}