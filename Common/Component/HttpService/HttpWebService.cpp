//
// Created by zmhy0073 on 2022/8/29.
//

#include"HttpWebService.h"
#include"Component/Lua/LuaScriptComponent.h"
namespace Sentry
{
    bool HttpWebService::OnStartService(HttpServiceRegister &serviceRegister)
    {
        serviceRegister.Bind("Sync", &HttpWebService::Sync);
        serviceRegister.Bind("Async", &HttpWebService::Async);
        this->mLuaComponent = this->GetComponent<LuaScriptComponent>();
        return this->GetApp()->GetConfig().GetPath("http", this->mPath);;
    }

    XCode HttpWebService::Sync(const HttpHandlerRequest &request, HttpHandlerResponse &response)
    {
        const std::string & path = request.GetData().mPath;
        lua_State * luaEnv = this->mLuaComponent->GetLuaEnv();
        const std::string file = path.substr(path.size() + 1);
        const std::string fullPath = this->mPath + file;
        if(luaL_loadfile(luaEnv, fullPath.c_str()) != LUA_OK)
        {
            response.AddHead("err", "load lua file error");
            return XCode::Failure;
        }
        request.GetData().Writer(luaEnv);
        if(lua_pcall(luaEnv, 1, 1, 0) != LUA_OK)
        {
            response.AddHead("err", lua_tostring(luaEnv, -1));
            return XCode::Failure;
        }
        if(lua_isstring(luaEnv, -1))
        {
            size_t size = 0;
            const char * str = luaL_checklstring(luaEnv, -1, &size);
            response.WriteString(str, size);
        }
        else if(lua_istable(luaEnv, -1))
        {

        }
        return XCode::Successful;
    }

    XCode HttpWebService::Async(const HttpHandlerRequest &request, HttpHandlerResponse &response)
    {
        return XCode::Successful;
    }

}