//
// Created by yjz on 2022/1/23.
//
#include"HttpService.h"
#include"Lua/Module/LuaModule.h"
#include"Util/Json/Lua/Json.h"
#include"Lua/Component/LuaComponent.h"
#include "Rpc/Lua/LuaServiceTaskSource.h"

namespace Tendo
{

    HttpService::HttpService()
		: mServiceRegister(this), mConfig(nullptr)
    {
		this->mLuaModule = nullptr;
    }

    bool HttpService::LateAwake()
    {
		const std::string & name = this->GetName();
		this->mConfig = HttpConfig::Inst()->GetConfig(name);
		LuaComponent * luaComponent = this->GetComponent<LuaComponent>();
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
				this->mLuaModule->AddCache(methodConfig->Method);
			}
			int code = this->mLuaModule->Call("Awake");
			return code != XCode::CallLuaFunctionFail;
		}
		return true;
    }

	void HttpService::Start()
	{
		this->OnStart();
		if(this->mLuaModule != nullptr)
		{
			this->mLuaModule->Await("Start");
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

	bool HttpService::OnHotFix()
	{
		if(this->mLuaModule != nullptr)
		{
			return this->mLuaModule->Hotfix();
		}
		return true;
	}

	void HttpService::Complete()
	{
		this->OnComplete();
		if(this->mLuaModule != nullptr)
		{
			this->mLuaModule->Await("Complete");
		}
	}

	void HttpService::OnDestroy()
	{
		this->OnStop();
		if(this->mLuaModule != nullptr)
		{
			this->mLuaModule->Await("Stop");
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
		this->mLuaModule->GetFunction("__Invoke");
		lua_State * lua = this->mLuaModule->GetLuaEnv();
		{
			request.WriteToLua(lua);
			if(lua_pcall(lua, 1, 3, 0) != LUA_OK)
			{
				Json::Writer document;
				int code = XCode::CallLuaFunctionFail;
				const char * err = lua_tostring(lua, -1);
				{
					document.Add("code").Add(code);
					document.Add("error").Add(err);
					LOG_ERROR(err);
				}
				response.Json(HttpStatus::OK, document);
				return XCode::CallLuaFunctionFail;
			}
			if (lua_istable(lua, -1))
			{
				std::string data;
				Lua::RapidJson::Read(lua, -1, &data);
				response.Json(HttpStatus::OK, data.c_str(), data.size());
				return (int)luaL_checkinteger(lua, -2);
			}
			return XCode::Failure;
		}
	}

	int HttpService::AwaitCallLua(const std::string & method, const Http::Request& request, Http::DataResponse& response)
	{
		this->mLuaModule->GetFunction("__Call");
		lua_State * lua = this->mLuaModule->GetLuaEnv();
		{
			lua_pushstring(lua, method.c_str());

			request.WriteToLua(lua);
			std::unique_ptr<LuaServiceTaskSource> luaTaskSource =
					std::make_unique<LuaServiceTaskSource>(&response);
			Lua::UserDataParameter::Write(lua, luaTaskSource.get());
			if (lua_pcall(lua, 4, 1, 0) != LUA_OK)
			{
				Json::Writer document;
				int code = XCode::CallLuaFunctionFail;
				const char * err = lua_tostring(lua, -1);
				{
					document.Add("code").Add(code);
					document.Add("error").Add(err);
					LOG_ERROR(err);
				}
				response.Json(HttpStatus::OK, document);
				return XCode::CallLuaFunctionFail;
			}
			return luaTaskSource->Await();
		}
	}
}