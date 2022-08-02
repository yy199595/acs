//
// Created by zmhy0073 on 2022/6/6.
//

#include"LuaHttpServiceMethod.h"
#include"Script/Function.h"
#include"Script/Extension/Json/Json.h"
#include"Async/LuaServiceTaskSource.h"
#include"Component/Scene/MessageComponent.h"
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
            Json::Writer data;
            data << "code " << (int)XCode::CallFunctionNotExist;
            data << "error" << "call lua function not existe";
            response.WriteString(data.JsonString());
            return XCode::CallFunctionNotExist;
        }
        request.GetData().Writer(this->mLua);
        std::shared_ptr<Json::Writer> jsonWriter = std::make_shared<Json::Writer>();
        XCode code = this->mConfig->IsAsync ? this->CallAsync(*jsonWriter) : this->Call(*jsonWriter);

        (*jsonWriter) << "code" << (int)code;
        response.WriteString(*jsonWriter);

        return code;
    }

    XCode LuaHttpServiceMethod::Call(Json::Writer &response)
    {
        if (lua_pcall(this->mLua, 1, 2, 0) != LUA_OK)
        {
            response << "error" << lua_tostring(this->mLua, -1);
            return XCode::CallLuaFunctionFail;
        }
        if (lua_isstring(this->mLua, -1))
        {
            size_t size = 0;
            const char *json = lua_tolstring(this->mLua, -1, &size);
            response << "data"; response.AddBinString(json, size);
            return XCode::Successful;
        }
        else if (lua_istable(this->mLua, -1))
        {
            this->mData.clear();
            Lua::Json::Read(this->mLua, -1, &this->mData);
            response << "data" << this->mData;
            return XCode::Successful;
        }
        return (XCode) lua_tointeger(this->mLua, -2);
    }

    XCode LuaHttpServiceMethod::CallAsync(Json::Writer &response)
    {
        LuaServiceTaskSource * luaTaskSource = new LuaServiceTaskSource(this->mLua);
        Lua::UserDataParameter::Write(this->mLua, luaTaskSource);
        if (lua_pcall(this->mLua, 3, 1, 0) != LUA_OK)
        {
            delete luaTaskSource;
            response << "error" << lua_tostring(this->mLua, -1);
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
                response << "data";
                response.AddBinString(json, size);
                return XCode::Successful;
            }
            if (lua_istable(this->mLua, -1))
            {
                this->mData.clear();
                Lua::Json::Read(this->mLua, -1, &this->mData);
                response << "data" << this->mData;
                return XCode::Successful;
            }
        }
        return code;
    }
}