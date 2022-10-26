//
// Created by zmhy0073 on 2022/8/29.
//
#include"HttpWebService.h"
#include"Math/MathHelper.h"
#include"App/System/System.h"
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

    XCode HttpWebService::Ping(const HttpHandlerRequest &request, HttpHandlerResponse &response)
    {
        response.Str(HttpStatus::OK,"pong");
        return XCode::Successful;
    }

	XCode HttpWebService::Sleep(const HttpHandlerRequest& request, HttpHandlerResponse& response)
	{
		int time = Helper::Math::Random<int>(500, 3000);
		this->mApp->GetTaskComponent()->Sleep(time);
		response.Str(HttpStatus::OK,fmt::format("sleep {0}ms", time));
		return XCode::Successful;
	}

    XCode HttpWebService::Hello(const HttpHandlerRequest &request, HttpHandlerResponse &response)
    {
		response.Str(HttpStatus::OK,"hello");
		return XCode::Successful;
    }

    XCode HttpWebService::DownLoad(const HttpHandlerRequest &request, HttpHandlerResponse &response)
    {
        return XCode::Successful;
    }

}