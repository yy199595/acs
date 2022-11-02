//
// Created by zmhy0073 on 2022/6/6.
//

#include"LuaHttpServiceMethod.h"
#include"Lua/Function.h"
#include"Json/Lua/Json.h"
#include"Json/Lua/values.hpp"
#include"Lua/LuaServiceTaskSource.h"
#include"Component/ProtoComponent.h"
namespace Sentry
{
    LuaHttpServiceMethod::LuaHttpServiceMethod(const HttpMethodConfig *config, lua_State *lua)
        : HttpServiceMethod(config->Method)
    {
        this->mLua = lua;
        this->mConfig = config;
    }

    XCode LuaHttpServiceMethod::Invoke(const Http::Request &request, Http::Response &response)
    {
        if(this->mConfig->IsAsync && !Lua::Function::Get(this->mLua, "coroutine", "http"))
        {
            return XCode::CallLuaFunctionFail;
        }
        const char * tab = this->mConfig->Service.c_str();
        const char * method = this->mConfig->Method.c_str();
        if(!Lua::Function::Get(this->mLua, tab, method))
		{
			Json::Document document;
			document.Add("error", "call lua function not existe");
			document.Add("code", (int)XCode::CallFunctionNotExist);
			response.Json(HttpStatus::OK, document);
			return XCode::CallFunctionNotExist;
		}
        rapidjson::Document document;
        request.WriteDocument(&document);
        values::pushValue(this->mLua, document);
        return this->mConfig->IsAsync ? this->CallAsync(response) : this->Call(response);
    }

    XCode LuaHttpServiceMethod::Call(Http::Response &response)
    {
        if (lua_pcall(this->mLua, 1, 2, 0) != LUA_OK)
        {
			Json::Document document;
			document.Add("error", lua_tostring(this->mLua, -1));
			document.Add("code", (int)XCode::CallLuaFunctionFail);
			response.Json(HttpStatus::OK, document);
			return XCode::CallLuaFunctionFail;
        }
        if (lua_isstring(this->mLua, -1))
        {
            size_t size = 0;
            const char *json = lua_tolstring(this->mLua, -1, &size);
			response.Json(HttpStatus::OK, json, size);
            return XCode::Successful;
        }
        else if (lua_istable(this->mLua, -1))
        {
            this->mData.clear();
            Lua::Json::Read(this->mLua, -1, &this->mData);
			response.Json(HttpStatus::OK, this->mData.c_str(), this->mData.size());
            return XCode::Successful;
        }
        return (XCode) lua_tointeger(this->mLua, -2);
    }

    XCode LuaHttpServiceMethod::CallAsync(Http::Response &response)
    {
        LuaServiceTaskSource * luaTaskSource = new LuaServiceTaskSource(this->mLua);
        Lua::UserDataParameter::Write(this->mLua, luaTaskSource);
        if (lua_pcall(this->mLua, 3, 1, 0) != LUA_OK)
        {
            delete luaTaskSource;
			Json::Document document;
			document.Add("error", lua_tostring(this->mLua, -1));
			document.Add("code", (int)XCode::CallLuaFunctionFail);
			response.Json(HttpStatus::OK, document);
            return XCode::CallLuaFunctionFail;
        }
        XCode code = luaTaskSource->Await();
        if (luaTaskSource->GetRef())
        {
            delete luaTaskSource;
            if (lua_isstring(this->mLua, -1))
            {
                size_t size = 0;
                const char *json = lua_tolstring(this->mLua, -1, &size);
				response.Json(HttpStatus::OK, json, size);
                return XCode::Successful;
            }
            if (lua_istable(this->mLua, -1))
            {
                this->mData.clear();
                Lua::Json::Read(this->mLua, -1, &this->mData);
				response.Json(HttpStatus::OK, this->mData.c_str(), this->mData.size());
                return XCode::Successful;
            }
        }
        return code;
    }
}