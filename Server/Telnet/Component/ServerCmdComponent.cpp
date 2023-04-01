//
// Created by yjz on 2023/4/2.
//

#include"ServerCmdComponent.h"
#include"Entity/App/App.h"
#include"Http/Common/HttpResponse.h"
#include"Http/Component/HttpComponent.h"
namespace Sentry
{
	bool ServerCmdComponent::Awake()
	{
		BIND_COMMAND("stop", ServerCmdComponent::Stop);
		this->mHttpComponent = this->mApp->GetComponent<HttpComponent>();
		return true;
	}

	void ServerCmdComponent::Help(std::stringstream& ss)
	{
		ss << "server stop : 关闭所有服务器";
	}

	void ServerCmdComponent::Stop(const std::string& args)
	{
		std::string url = fmt::format("http://{0}/server/stop", args);
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