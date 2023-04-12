//
// Created by zmhy0073 on 2022/6/6.
//

#include"LuaHttpServiceMethod.h"
#include"Lua/Engine/Function.h"
#include"Util/Json/Lua/Json.h"
#include"Lua/Module/LuaModule.h"
#include"Rpc/Lua/LuaServiceTaskSource.h"
#include"Proto/Component/ProtoComponent.h"
#include"Lua/Component/LuaScriptComponent.h"

namespace Tendo
{
    LuaHttpServiceMethod::LuaHttpServiceMethod(const HttpMethodConfig *config)
        : HttpServiceMethod(config->Method)
    {      
        this->mConfig = config;
        this->mLuaComponent = App::Inst()->GetComponent<LuaScriptComponent>();
    }

    int LuaHttpServiceMethod::Invoke(const Http::Request &request, Http::DataResponse &response)
    {
        lua_State* lua = this->mLuaComponent->GetLuaEnv();
        if(this->mConfig->IsAsync && !Lua::Function::Get(lua, "coroutine", "http"))
        {
            return XCode::CallLuaFunctionFail;
        }
        const std::string& service = this->mConfig->Service;
        Lua::LuaModule * luaModule = this->mLuaComponent->GetModule(service);
        if (luaModule == nullptr || !luaModule->GetFunction(this->mConfig->Method))
        {
            Json::Writer document;
            document.Add("error").Add("call lua function not exist");
            document.Add("code").Add((int)XCode::CallFunctionNotExist);
            response.Json(HttpStatus::OK, document);
            return XCode::CallFunctionNotExist;          
        }
        if (!request.WriteLua(lua))
        {
            lua_pushnil(lua);
        }
        return this->mConfig->IsAsync ? this->CallAsync(response) : this->Call(response);
    }

    int LuaHttpServiceMethod::Call(Http::DataResponse &response)
    {
        lua_State* lua = this->mLuaComponent->GetLuaEnv();
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
        return lua_tointeger(lua, -2);
    }

    int LuaHttpServiceMethod::CallAsync(Http::DataResponse &response)
    {
        lua_State* lua = this->mLuaComponent->GetLuaEnv();       
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