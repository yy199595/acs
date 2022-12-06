//
// Created by zmhy0073 on 2022/8/29.
//
#include"HttpBackGround.h"
#include"System/System.h"
#include"Config/CodeConfig.h"
#include"Service/InnerService.h"
#include"Component/LocationComponent.h"
namespace Sentry
{
    bool HttpBackGround::OnStartService(HttpServiceRegister &serviceRegister)
    {
        serviceRegister.Bind("Info", &HttpBackGround::Info);
        serviceRegister.Bind("Ping", &HttpBackGround::Ping);
        serviceRegister.Bind("Hello", &HttpBackGround::Hello);
		serviceRegister.Bind("Sleep", &HttpBackGround::Sleep);
		serviceRegister.Bind("Hotfix", &HttpBackGround::Hotfix);
		serviceRegister.Bind("DownLoad", &HttpBackGround::DownLoad);
        return true;
    }

    XCode HttpBackGround::Ping(const Http::Request &request, Http::Response &response)
    {
        response.Str(HttpStatus::OK,"pong");
        return XCode::Successful;
    }

	XCode HttpBackGround::Hotfix(Json::Document &response)
	{
		InnerService * innerService = this->GetComponent<InnerService>();
		LocationComponent * locationComponent = this->GetComponent<LocationComponent>();
		if(locationComponent == nullptr || innerService == nullptr)
		{
			response.Add("error", "LocationComponent or InnerService Is Null");
			return XCode::Failure;
		}
		std::vector<std::string> locations;
		locationComponent->GetServers(locations);
		for(const std::string & location : locations)
		{
			XCode code = innerService->Call(location, "Hotfix");
            response.Add(location.c_str(), CodeConfig::Inst()->GetDesc(code));
		}
		return XCode::Successful;
	}

	XCode HttpBackGround::Sleep(const Json::Reader &request, Json::Document &response)
    {
        std::string time;
        request.GetMember("time", time);
        this->mApp->GetTaskComponent()->Sleep(std::stoi(time));
        response.Add("time", std::stoi(time));
        return XCode::Successful;
    }

    XCode HttpBackGround::Hello(const Http::Request &request, Http::Response &response)
    {
		response.Str(HttpStatus::OK,"hello");
		return XCode::Successful;
    }

    XCode HttpBackGround::DownLoad(const Http::Request &request, Http::Response &response)
    {
        return XCode::Successful;
    }

    XCode HttpBackGround::Info(Json::Document &response)
    {
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
                std::unique_ptr<Json::Document> document1(new Json::Document());
                serverRecord->OnRecord(*document1);
                const char* key = component->GetName().c_str();
                response.Add(key, std::move(document1));
            }
        }
        return XCode::Successful;
    }

}