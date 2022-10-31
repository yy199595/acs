//
// Created by zmhy0073 on 2022/8/29.
//
#include"HttpWebService.h"
#include"Math/MathHelper.h"
#include"System/System.h"
#include"Component/LuaScriptComponent.h"
namespace Sentry
{
    bool HttpWebService::OnStartService(HttpServiceRegister &serviceRegister)
    {
        serviceRegister.Bind("Ping", &HttpWebService::Ping);
        serviceRegister.Bind("Hello", &HttpWebService::Hello);
		serviceRegister.Bind("Sleep", &HttpWebService::Sleep);
		serviceRegister.Bind("DownLoad", &HttpWebService::DownLoad);
        return true;
    }

    XCode HttpWebService::Ping(const Http::Request &request, Http::Response &response)
    {
        response.Str(HttpStatus::OK,"pong");
        return XCode::Successful;
    }

	XCode HttpWebService::Sleep(const Json::Reader &request, Json::Document &response)
    {
        std::string time;
        request.GetMember("time", time);
        this->mApp->GetTaskComponent()->Sleep(std::stoi(time));
        response.Add("time", std::stoi(time));
        return XCode::Successful;
    }

    XCode HttpWebService::Hello(const Http::Request &request, Http::Response &response)
    {
		response.Str(HttpStatus::OK,"hello");
		return XCode::Successful;
    }

    XCode HttpWebService::DownLoad(const Http::Request &request, Http::Response &response)
    {
        return XCode::Successful;
    }

}