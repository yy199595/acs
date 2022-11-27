//
// Created by zmhy0073 on 2022/6/6.
//

#include"LuaHttpServiceMethod.h"
#include"Lua/Function.h"
#include"Json/Lua/Json.h"
#include"Json/Lua/values.hpp"
#include"Lua/LuaServiceTaskSource.h"
#include"Component/ProtoComponent.h"
#include"Component/LuaScriptComponent.h"

namespace Sentry
{
    LuaHttpServiceMethod::LuaHttpServiceMethod(const HttpMethodConfig *config)
        : HttpServiceMethod(config->Method)
    {      
        this->mConfig = config;
        this->mLuaComponent = App::Inst()->GetComponent<LuaScriptComponent>();
    }

    XCode LuaHttpServiceMethod::Invoke(const Http::Request &request, Http::Response &response)
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
            Json::Document document;
            document.Add("error", "call lua function not existe");
            document.Add("code", (int)XCode::CallFunctionNotExist);
            response.Json(HttpStatus::OK, document);
            return XCode::CallFunctionNotExist;          
        }       
        rapidjson::Document document;
        request.WriteDocument(&document);
        values::pushValue(lua, document);
        return this->mConfig->IsAsync ? this->CallAsync(response) : this->Call(response);
    }

    XCode LuaHttpServiceMethod::Call(Http::Response &response)
    {
        lua_State* lua = this->mLuaComponent->GetLuaEnv();
        if (lua_pcall(lua, 1, 2, 0) != LUA_OK)
        {
			Json::Document document;
			document.Add("error", lua_tostring(lua, -1));
			document.Add("code", (int)XCode::CallLuaFunctionFail);
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
            this->mData.clear();
            Lua::Json::Read(lua, -1, &this->mData);
			response.Json(HttpStatus::OK, this->mData.c_str(), this->mData.size());
            return XCode::Successful;
        }
        return (XCode) lua_tointeger(lua, -2);
    }

    XCode LuaHttpServiceMethod::CallAsync(Http::Response &response)
    {
        lua_State* lua = this->mLuaComponent->GetLuaEnv();
        auto * luaTaskSource = new LuaServiceTaskSource(lua);
        Lua::UserDataParameter::Write(lua, luaTaskSource);
        if (lua_pcall(lua, 3, 1, 0) != LUA_OK)
        {           
			Json::Document document;
			document.Add("error", lua_tostring(lua, -1));
			document.Add("code", (int)XCode::CallLuaFunctionFail);
			response.Json(HttpStatus::OK, document);

            delete luaTaskSource;
            return XCode::CallLuaFunctionFail;
        }
        XCode code = luaTaskSource->Await();
        if (luaTaskSource->GetRef())
        {           
            if (lua_isstring(lua, -1))
            {

                size_t size = 0;
                const char *json = lua_tolstring(lua, -1, &size);
				response.Json(HttpStatus::OK, json, size);

                delete luaTaskSource;
                return XCode::Successful;
            }
            if (lua_istable(lua, -1))
            {
                this->mData.clear();
                Lua::Json::Read(lua, -1, &this->mData);
				response.Json(HttpStatus::OK, this->mData.c_str(), this->mData.size());
                delete luaTaskSource;
                return XCode::Successful;
            }
        }
        delete luaTaskSource;
        return code;
    }
}