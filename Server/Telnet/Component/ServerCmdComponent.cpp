//
// Created by yjz on 2023/4/2.
//

#include"ServerCmdComponent.h"
#include"Entity/App/App.h"
#include"Util/String/StringHelper.h"
#include"Http/Common/HttpResponse.h"
#include"Http/Component/HttpComponent.h"
namespace Tendo
{
	bool ServerCmdComponent::Awake()
	{
		BIND_COMMAND("stop", ServerCmdComponent::Stop);
		BIND_COMMAND("info", ServerCmdComponent::Info);
		this->mHttpComponent = this->mApp->GetComponent<HttpComponent>();
		return true;
	}

	void ServerCmdComponent::Help(std::stringstream& ss)
	{
		ss << "server stop + http address	: stop all server" << "\n";
		ss << "server info + http address	: get server run info" << "\n";
	}

	void ServerCmdComponent::Info(const std::string& args)
	{
		if(args.empty())
		{
			Debug::Print(spdlog::level::err, "input http address ip:port");
			return;
		}
		std::string url = fmt::format("http://{0}/server/info", args);
		Debug::Print(spdlog::level::debug, "start request url :" + url);
		std::shared_ptr<Http::DataResponse> response = this->mHttpComponent->Get(url, 100.0);
		if(response == nullptr || response->Code() != HttpStatus::OK)
		{
			Debug::Print(spdlog::level::err, "invoke command failure");
		}
		else
		{
			const std::string & json = response->GetContent();
			Debug::Print(spdlog::level::info, Helper::Str::FormatJson(json));
		}
	}

	void ServerCmdComponent::Stop(const std::string& args)
	{
		if(args.empty())
		{		
			Debug::Print(spdlog::level::err, "input http address ip:port");
			return;
		}
		std::string url = fmt::format("http://{0}/server/stop", args);
		Debug::Print(spdlog::level::debug, "start request url :" + url);
		std::shared_ptr<Http::DataResponse> response = this->mHttpComponent->Get(url, 100.0);
		if(response == nullptr || response->Code() != HttpStatus::OK)
		{
			CONSOLE_LOG_ERROR("use http stop server error");
		}
		else
		{
			CONSOLE_LOG_INFO(response->GetContent());
		}
	}
}