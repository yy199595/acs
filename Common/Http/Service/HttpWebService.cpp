//
// Created by zmhy0073 on 2022/8/29.
//
#include<fstream>
#include"HttpWebService.h"
#include"File/FileHelper.h"
#include"Json/Lua/Json.h"
#include"Component/LuaScriptComponent.h"
namespace Sentry
{
    bool HttpWebService::OnStartService(HttpServiceRegister &serviceRegister)
    {
        const ServerConfig & config = this->GetApp()->GetConfig();
        this->mLuaComponent = this->GetComponent<LuaScriptComponent>();
        serviceRegister.Bind("Sync", &HttpWebService::Sync);
        serviceRegister.Bind("Async", &HttpWebService::Async);
        serviceRegister.Bind("DownLoad", &HttpWebService::DownLoad);
        LOG_CHECK_RET_FALSE(config.GetMember("web", "lua", this->mLuaPath));
        LOG_CHECK_RET_FALSE(config.GetMember("web", "download", this->mDownloadPath));

        this->mLuaPath = config.GetWorkPath() + this->mLuaPath;
        this->mDownloadPath = config.GetWorkPath() + this->mDownloadPath;
        return true;
    }

    XCode HttpWebService::Sync(const HttpHandlerRequest &request, HttpHandlerResponse &response)
    {
        const HttpData &httpData = request.GetData();
        std::string path = this->mLuaPath + httpData.mPath;
        lua_State *luaEnv = this->mLuaComponent->GetLuaEnv();
        if (luaL_loadfile(luaEnv, path.c_str()) != LUA_OK)
        {
            response.WriteString(lua_tostring(luaEnv, -1));
            return XCode::Failure;
        }
        httpData.Writer(luaEnv);
        if(lua_pcall(luaEnv, 1, 1, 0) != LUA_OK)
        {
            response.WriteString(lua_tostring(luaEnv, -1));
            return XCode::Failure;
        }
        if (lua_isstring(luaEnv, -1))
        {
            size_t size = 0;
            const char *str = luaL_checklstring(luaEnv, -1, &size);
            response.WriteString(str, size);
        }
        else if (lua_istable(luaEnv, -1))
        {
            std::string json;
            Lua::Json::Read(luaEnv, -1, &json);
            response.WriteString(json);
        }
        return XCode::Successful;
    }

    XCode HttpWebService::Async(const HttpHandlerRequest &request, HttpHandlerResponse &response)
    {
        return XCode::Successful;
    }

    XCode HttpWebService::DownLoad(const HttpHandlerRequest &request, HttpHandlerResponse &response)
    {
        const HttpData & httpData = request.GetData();
        std::string path = this->mDownloadPath + httpData.mPath;
        std::fstream * fs = new std::fstream(path.c_str());
        if(!fs->is_open())
        {
            delete fs;
            return XCode::CallServiceNotFound;
        }
        response.WriteFile(fs);
        return XCode::Successful;
    }

}