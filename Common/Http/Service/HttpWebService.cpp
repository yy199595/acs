//
// Created by zmhy0073 on 2022/8/29.
//
#include"HttpWebService.h"
#include"System/System.h"
#include"Config/CodeConfig.h"
#include"Service/InnerService.h"
#include"Component/LocationComponent.h"
namespace Sentry
{
    bool HttpWebService::OnStartService(HttpServiceRegister &serviceRegister)
    {
        serviceRegister.Bind("Info", &HttpWebService::Info);
        serviceRegister.Bind("Ping", &HttpWebService::Ping);
        serviceRegister.Bind("Hello", &HttpWebService::Hello);
		serviceRegister.Bind("Sleep", &HttpWebService::Sleep);
		serviceRegister.Bind("Hotfix", &HttpWebService::Hotfix);
		serviceRegister.Bind("DownLoad", &HttpWebService::DownLoad);
        return true;
    }

    XCode HttpWebService::Ping(const Http::Request &request, Http::Response &response)
    {
        response.Str(HttpStatus::OK,"pong");
        return XCode::Successful;
    }

	XCode HttpWebService::Hotfix(const Http::Request& request, Http::Response& response)
	{
		InnerService * innerService = this->GetComponent<InnerService>();
		LocationComponent * locationComponent = this->GetComponent<LocationComponent>();
		if(locationComponent == nullptr || innerService == nullptr)
		{
			response.Str(HttpStatus::OK, "LocationComponent or InnerService Is Null");
			return XCode::Failure;
		}
		Json::Document jsonDocuemnt;
		std::vector<std::string> locations;
		locationComponent->GetServers(locations);
		for(const std::string & location : locations)
		{
			XCode code = innerService->Call(location, "Hotfix");
			jsonDocuemnt.Add(location.c_str(), CodeConfig::Inst()->GetDesc(code));
		}
		response.Json(HttpStatus::OK, jsonDocuemnt);
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

    XCode HttpWebService::Info(const Http::Request &request, Http::Response &response)
    {
        Json::Document document;
        std::vector<Component *> components;
        this->mApp->GetComponents(components);
        for(Component * component : components)
        {
            IServerRecord * serverRecord = component->Cast<IServerRecord>();
            if(serverRecord != nullptr)
            {
                IServiceBase * serviceBase = component->Cast<IServiceBase>();
                if(serviceBase != nullptr && !serviceBase->IsStartService())
                {
                    continue;
                }
                Json::Document document1;
                serverRecord->OnRecord(document1);
                document.Add(component->GetName().c_str(), document1);
            }
        }
        response.Json(HttpStatus::OK, document);
        return XCode::Successful;
    }

}