//
// Created by zmhy0073 on 2022/8/29.
//
#include"HttpBackGround.h"
#include"System/System.h"
#include"Config/CodeConfig.h"
#include"Service/Node.h"
#include"google/protobuf/wrappers.pb.h"
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

	XCode HttpBackGround::Hotfix(Json::Writer&response)
	{
		Node * innerService = this->GetComponent<Node>();
		LocationComponent * locationComponent = this->GetComponent<LocationComponent>();
		if(locationComponent == nullptr || innerService == nullptr)
		{
			response.Add("error").Add("LocationComponent or InnerService Is Null");
			return XCode::Failure;
		}
		std::vector<std::string> locations;
		locationComponent->GetServers(locations);
		for(const std::string & location : locations)
		{
			XCode code = innerService->Call(location, "Hotfix");
            response.Add(location.c_str()).Add(CodeConfig::Inst()->GetDesc(code));
		}
		return XCode::Successful;
	}

	XCode HttpBackGround::Sleep(const Json::Reader &request, Json::Writer&response)
    {
        std::string time;
        request.GetMember("time", time);
        this->mApp->GetTaskComponent()->Sleep(std::stoi(time));
        response.Add("time").Add(std::stoi(time));
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

    XCode HttpBackGround::Info(Json::Writer&response)
    {
		Node * innerService = this->GetComponent<Node>();
		LocationComponent * locationComponent = this->GetComponent<LocationComponent>();
		if(locationComponent == nullptr)
		{
			response.Add("error").Add("LocationComponent or InnerService Is Null");
			return XCode::Failure;
		}
		std::vector<std::string> locations;
		locationComponent->GetServers(locations);
		for(const std::string & location : locations)
		{
			std::shared_ptr<google::protobuf::StringValue> resp
				= std::make_shared<google::protobuf::StringValue>();
			XCode code = innerService->Call(location, "RunInfo", resp);
			if(code == XCode::Successful)
			{
				rapidjson::Document document;
				const std::string & json = resp->value();				
				if(!document.Parse(json.c_str(), json.size()).HasParseError())
				{
					response.Add(location.c_str()).Add(document);
				}
			}
			else
			{
				std::string desc = CodeConfig::Inst()->GetDesc(code);
				response.Add(location.c_str()).Add(desc);
			}
		}
		return XCode::Successful;
    }

}