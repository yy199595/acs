#include"ConsoleComponent.h"
#include"Component/Scene/OperatorComponent.h"
#include"Component/RpcService/RemoteServiceComponent.h"
#include"Component/Coroutine/TaskComponent.h"
#include"Component/RpcService/LocalServiceComponent.h"
#include"Network/Listener/TcpServerComponent.h"
#define BIND_FUNC(name, func) this->mFunctionMap.emplace(name, std::bind(&func, this, args1, args2));

namespace Sentry
{
	bool ConsoleComponent::LateAwake()
	{
		BIND_FUNC("help", ConsoleComponent::Help);
		BIND_FUNC("close", ConsoleComponent::Close);
		BIND_FUNC("hotfix", ConsoleComponent::Hotfix);
		BIND_FUNC("start", ConsoleComponent::Start);
		BIND_FUNC("offset", ConsoleComponent::Offset);
		BIND_FUNC("service", ConsoleComponent::Services);
		this->mTaskComponent = this->GetComponent<TaskComponent>();
		return true;
	}

	bool ConsoleComponent::OnStart()
	{
		TcpServerComponent * tcpComponent = this->GetComponent<TcpServerComponent>();
		return tcpComponent->StartListen("console");
	}

	void ConsoleComponent::OnListen(std::shared_ptr<SocketProxy> socket)
	{
		std::shared_ptr<TelnetClient> telnetClient(new TelnetClient(socket));
		this->mTaskComponent->Start(&ConsoleComponent::HandleConsoleClient, this, telnetClient);
	}

	void ConsoleComponent::HandleConsoleClient(std::shared_ptr<TelnetClient> telnetClient)
	{
		telnetClient->Response("welcome to sentry server");
		while (telnetClient->IsOpen())
		{
			std::shared_ptr<TelnetContent> commandContent = telnetClient->ReadCommand();
			LOG_INFO("==== console command ====");
			LOG_INFO("command = " << commandContent->Command);
			if (!commandContent->Parameter.empty())
			{
				LOG_INFO("parameter = " << commandContent->Parameter);
			}
			if (commandContent->IsOk)
			{
				std::stringstream responseStream;
				auto iter = this->mFunctionMap.find(commandContent->Command);
				if (iter == this->mFunctionMap.end())
				{
					responseStream << "\r\n" << "<CMD NOT>";
				}
				else
				{
					std::vector<std::string> response;
					ConsoleFunction function = iter->second;
					if (function(commandContent->Parameter, response))
					{
						for (const std::string& str : response)
						{
							responseStream << "\r\n" << str;
						}
						responseStream << "\r\n<CMD OK>";
					}
					else
					{
						responseStream << "\r\n<CMD ERR>";
					}
				}
				std::string res = responseStream.str();
				telnetClient->Response(responseStream.str());
			}
		}
		LOG_ERROR("[console ] " << telnetClient->GetAddress() << " disconnected");
	}

	bool ConsoleComponent::Help(const std::string& parameter, std::vector<std::string>& response)
	{
		auto iter = this->mFunctionMap.begin();
		for (; iter != this->mFunctionMap.end(); iter++)
		{
			response.emplace_back(iter->first);
		}
		return true;
	}

	bool ConsoleComponent::Start(const std::string& parameter, std::vector<std::string>& response)
	{
		return this->GetApp()->StartNewService(parameter);
	}

	bool ConsoleComponent::Close(const std::string& parameter, std::vector<std::string>& response)
	{
		App::Get()->Stop();
		return true;
	}

	bool ConsoleComponent::Services(const std::string& parameter, std::vector<std::string>& response)
	{
		std::vector<std::string> components;
		this->GetApp()->GetComponents(components);
		for(const std::string & name : components)
		{
			if(this->GetComponent<LocalServiceComponent>(name) != nullptr)
			{
				response.emplace_back(name);
			}
		}
		return true;
	}

	bool ConsoleComponent::Hotfix(const std::string& parameter, std::vector<std::string>& response)
	{
		auto operComponent = this->GetComponent<OperatorComponent>();
		operComponent->StartHotfix();
		return true;
	}

	bool ConsoleComponent::Offset(const string& parameter, vector<string>& response)
	{
		long long value = std::stoll(parameter);
		Helper::Time::SetScaleTotalTime(value);
		LOG_WARN("now time = " << Helper::Time::GetDateString());
		return true;
	}
}