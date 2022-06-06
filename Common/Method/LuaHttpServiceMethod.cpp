//
// Created by zmhy0073 on 2022/6/6.
//

#include"LuaHttpServiceMethod.h"
#include"Script/Function.h"
#include"Script/Extension/Json/Json.h"
#include"Async/LuaServiceTaskSource.h"
namespace Sentry
{
    LuaHttpServiceMethod::LuaHttpServiceMethod(const HttpInterfaceConfig *config, lua_State *lua)
        : HttpServiceMethod(config->Method)
    {
        this->mLua = lua;
        this->mConfig = config;
    }

    XCode LuaHttpServiceMethod::Invoke(const HttpHandlerRequest &request, HttpHandlerResponse &response)
    {
        if(this->mConfig->IsAsync && !Lua::Function::Get(this->mLua, "HttpCall"))
        {
            return XCode::CallLuaFunctionFail;
        }
        const char * tab = this->mConfig->Service.c_str();
        const char * method = this->mConfig->Method.c_str();
        if(!Lua::Function::Get(this->mLua, tab, method))
        {
            response.AddHead("error", "call function not existe");
            return XCode::CallFunctionNotExist;
        }
        this->mJson.clear();
        if(!request.ToJson(this->mJson))
        {
            response.AddHead("error", "cast json error");
            return XCode::Failure;
        }
        Lua::Json::Write(this->mLua, this->mJson);
        return this->mConfig->IsAsync ? this->CallAsync(response) : this->Call(response);
    }

    XCode LuaHttpServiceMethod::Call(HttpHandlerResponse &response)
    {
        if(lua_pcall(this->mLua, 1, 1, 0) != 0)
        {
            response.AddHead("error", lua_tostring(this->mLua, -1));
            return XCode::CallLuaFunctionFail;
        }
        if(lua_isstring(this->mLua, -1))
        {
            size_t size = 0;
            const char * json = lua_tolstring(this->mLua, -1, &size);
            response.WriteString(json, size);
            return XCode::Successful;
        }
        if(lua_istable(this->mLua, -1))
        {
            this->mJson.clear();
            Lua::Json::Read(this->mLua, -1, &mJson);
            response.WriteString(this->mJson);
            return XCode::Successful;
        }
        response.WriteString("unknow error");
        return XCode::Failure;
    }

    XCode LuaHttpServiceMethod::CallAsync(HttpHandlerResponse &response)
    {
		std::shared_ptr<LuaServiceTaskSource> luaTaskSource(new LuaServiceTaskSource(this->mLua));
		Lua::UserDataParameter::Write(this->mLua, luaTaskSource);
		if(lua_pcall(this->mLua, 3, 1, 0) != 0)
		{
			response.AddHead("error", lua_tostring(this->mLua, -1));
			return XCode::CallLuaFunctionFail;
		}
		XCode code = luaTaskSource->Await();
		if(code != XCode::Successful)
		{
			return code;
		}
		if(luaTaskSource->GetRef())
		{
			if (lua_isstring(this->mLua, -1))
			{
				size_t size = 0;
				const char* json = lua_tolstring(this->mLua, -1, &size);
				response.WriteString(json, size);
				return XCode::Successful;
			}
			if (lua_istable(this->mLua, -1))
			{
				this->mJson.clear();
				Lua::Json::Read(this->mLua, -1, &mJson);
				response.WriteString(this->mJson);
				return XCode::Successful;
			}
		}
		return code;
	}
}