//
// Created by zmhy0073 on 2022/8/29.
//
#include"ServerWeb.h"
#include"Message/s2s/s2s.pb.h"
#include"Core/System/System.h"
#include"Entity/Unit/App.h"
#include"Server/Config/CodeConfig.h"
#include"Common/Service/Node.h"
#include"Util/File/DirectoryHelper.h"
#include"Util/File/FileHelper.h"
#include"Rpc/Component/LocationComponent.h"
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
		LOG_CHECK_RET_FALSE(this->GetComponent<LocationComponent>());
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
		const ServerConfig * config = ServerConfig::Inst();
		RegistryComponent * pRegistryComponent = this->GetComponent<RegistryComponent>();
		LocationComponent * pLocationComponent = this->GetComponent<LocationComponent>();

		pRegistryComponent->Query();
		std::vector<ServerUnit *> servers;
		pLocationComponent->GetAllServer(servers);
		for(ServerUnit * locationUnit : servers)
		{
			const std::string func("Node.Stop");
			if(locationUnit->GetId() != config->ServerId())
			{

			}
		}
		this->mApp->GetCoroutine()->Start(&App::Stop, this->mApp, 0);
		return XCode::Successful;
	}

	int ServerWeb::Info(Json::Writer&response)
    {
		RegistryComponent * pRegistryComponent = this->GetComponent<RegistryComponent>();
		LocationComponent * pLocationComponent = this->GetComponent<LocationComponent>();

		pRegistryComponent->Query();
		std::vector<ServerUnit *> servers;
		pLocationComponent->GetAllServer(servers);
		response.BeginArray("list");
		for(ServerUnit * locationUnit : servers)
		{
			int id = locationUnit->GetId();
			std::string rpc, http, gate;
			locationUnit->Get("rpc", rpc);
			locationUnit->Get("http", http);
			locationUnit->Get("gate", gate);
			response.BeginObject();
			response.Add("id").Add(id);
			response.Add("name").Add(locationUnit->Name());
			response.Add("rpc").Add(rpc).Add("http").Add(http).Add("gate").Add(gate);
			response.EndObject();
		}
		response.EndArray();
		return XCode::Successful;
    }
}