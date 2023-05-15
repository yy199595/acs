#include<iostream>
#include"ConsoleComponent.h"
#include"Entity/Unit/App.h"
#include"Entity/Unit/Unit.h"
#include"Http/Component/HttpComponent.h"
#include"Async/Component/CoroutineComponent.h"

#include"Telnet/Component/ServerCmdComponent.h"
namespace Tendo
{
	void TelnetProto::Add(const std::string& content)
	{
		this->mContents.emplace_back(content);
	}

	void TelnetProto::Add(const char* str, size_t size)
	{
		this->mContents.emplace_back(str, size);
	}

	int TelnetProto::Serialize(std::ostream& os)
	{
		for(const std::string & str : this->mContents)
		{
			os << str << "\r\n";
		}
		return 0;
	}
}

using namespace Tcp;
namespace Tendo
{
	void ConsoleComponent::OnClusterComplete()
	{
		this->mCommandUnit; //= new Unit(1);
		this->mHttpComponent = this->GetComponent<HttpComponent>();
		this->mTaskComponent = this->GetComponent<CoroutineComponent>();
		this->mTaskComponent->Start(&ConsoleComponent::Update, this);

		this->mCommandUnit->AddComponent("server", std::make_unique<ServerCmdComponent>());
	}

	void ConsoleComponent::Close(const std::string & request)
	{

	}
	void ConsoleComponent::Update()
	{
		while(true)
		{
			Debug::Print(spdlog::level::debug, "please input cmd");
			std::string line;
			std::getline(std::cin, line);
			if(line == "quit")
			{
				this->mApp->Stop(0);
				CONSOLE_LOG_INFO("close console successful");
				break;
			}
			else if(line == "help")
			{
				std::stringstream ss;
				std::vector<ConsoleCmdComponent *> components;
				this->mCommandUnit->GetComponents(components);
				for(ConsoleCmdComponent * consoleCmdComponent : components)
				{
					consoleCmdComponent->Help(ss);
					ss << "\n";
				}
				Debug::Print(Debug::Level::info, ss.str());
				break;
			}
			std::string command;
			std::string request1;
			std::string request2;
			size_t pos = line.find(' ');
			if(pos != std::string::npos)
			{
				command = line.substr(0, pos);
				request1 = line.substr(pos + 1);
				pos = request1.find(' ');
				if(pos != std::string::npos)
				{
					request2 = request1.substr(pos+ 1);
					request1 = request1.substr(0, pos);
				}
			}
			ConsoleCmdComponent * commandComponent =
				this->mCommandUnit->GetComponent<ConsoleCmdComponent>(command);
			if(commandComponent == nullptr || !commandComponent->Invoke(request1, request2))
			{
				std::stringstream ss;
				ss << "unknown command [" << command << " " << request1 << "]";
				Debug::Print(Debug::Level::err, ss.str());
			}
		}
	}
}