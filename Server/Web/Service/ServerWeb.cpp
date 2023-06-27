//
// Created by zmhy0073 on 2022/8/29.
//
#include"ServerWeb.h"
#include"Message/s2s/s2s.pb.h"
#include"Core/System/System.h"
#include"Entity/Actor/App.h"
#include"Util/File/DirectoryHelper.h"
#include"Registry/Component/RegistryComponent.h"
namespace Tendo
{
    bool ServerWeb::OnInit()
	{
		BIND_COMMON_HTTP_METHOD(ServerWeb::Info);
		BIND_COMMON_HTTP_METHOD(ServerWeb::Stop);
		BIND_COMMON_HTTP_METHOD(ServerWeb::Login);
		BIND_COMMON_HTTP_METHOD(ServerWeb::Hotfix);
		BIND_COMMON_HTTP_METHOD(ServerWeb::Register);
		LOG_CHECK_RET_FALSE(this->GetComponent<RegistryComponent>());
		return true;
	}

    int ServerWeb::Login(const Http::Request &request, Http::DataResponse &response)
    {
        std::string account,password;
        Http::Parameter parameter(request.Content());
		LOG_ERROR_CHECK_ARGS(parameter.Get("account,", account));
		LOG_ERROR_CHECK_ARGS(parameter.Get("password", password));
        return XCode::Successful;
    }

    int ServerWeb::Register(const Http::Request& request, Http::DataResponse& response)
    {
        return XCode::Successful;
    }

	int ServerWeb::Hotfix(Json::Writer&response)
	{

		return XCode::Successful;
	}

	int ServerWeb::Stop(Json::Writer & response)
	{
		this->mApp->GetCoroutine()->Start(&App::Stop, this->mApp, 0);
		return XCode::Successful;
	}

	int ServerWeb::Info(Json::Writer&response)
    {
		return XCode::Successful;
    }
}