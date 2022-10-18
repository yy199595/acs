//
// Created by zmhy0073 on 2022/8/29.
//
#include<fstream>
#include"HttpWebService.h"
#include"Json/Lua/Json.h"
#include"App/System/System.h"
#include"Component/HttpWebComponent.h"
#include"Component/LuaScriptComponent.h"
namespace Sentry
{
    bool HttpWebService::Awake()
    {
        return this->mApp->AddComponent<HttpWebComponent>();
    }

    bool HttpWebService::OnStartService(HttpServiceRegister &serviceRegister)
    {
        const ServerConfig * localServerConfig = ServerConfig::Inst();
        this->mLuaComponent = this->GetComponent<LuaScriptComponent>();
        serviceRegister.Bind("Ping", &HttpWebService::Ping);
        serviceRegister.Bind("Hello", &HttpWebService::Hello);
        serviceRegister.Bind("DownLoad", &HttpWebService::DownLoad);
        LOG_CHECK_RET_FALSE(localServerConfig->GetMember("web", "lua", this->mLuaPath));
        LOG_CHECK_RET_FALSE(localServerConfig->GetMember("web", "download", this->mDownloadPath));

        this->mLuaPath = System::GetWorkPath() + this->mLuaPath;
        this->mDownloadPath = System::GetWorkPath() + this->mDownloadPath;
        return this->GetComponent<HttpWebComponent>()->StartListen("web");
    }

    XCode HttpWebService::Ping(const HttpHandlerRequest &request, HttpHandlerResponse &response)
    {
        response.WriteString("pong");
        return XCode::Successful;
    }

    XCode HttpWebService::Hello(const HttpHandlerRequest &request, HttpHandlerResponse &response)
    {
        response.WriteString("hello");
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