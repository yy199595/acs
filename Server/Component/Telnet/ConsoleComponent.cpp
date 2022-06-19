#include"ConsoleComponent.h"
#include"Component/Scene/OperatorComponent.h"
#include"Component/RpcService/ServiceComponent.h"
#include"Component/Coroutine/TaskComponent.h"
#include"Component/RpcService/LocalServiceComponent.h"
#include"Network/Listener/TcpServerComponent.h"
#define BIND_FUNC(name, func) this->mFunctionMap.emplace(name, std::bind(&func, this, args1, args2));

namespace Sentry
{
	void TelnetProto::Add(const std::string& conetnt)
	{
		this->mContents.emplace_back(conetnt);
	}

	void TelnetProto::Add(const char* str, size_t size)
	{
		this->mContents.emplace_back(str, size);
	}

	int TelnetProto::Serailize(std::ostream& os)
	{
		for(const std::string & str : this->mContents)
		{
			os << str << "\r\n";
		}
		return 0;
	}
}

using namespace Tcp;
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
		std::shared_ptr<TelnetClientContext> telnetClient =
			std::make_shared<TelnetClientContext>(socket, this);

		std::shared_ptr<TelnetProto> telnetProto(new TelnetProto());
		telnetProto->Add("welcome connect sertry server");


		telnetClient->StartRead();
		this->mTelnetClients.emplace(telnetClient);
		telnetClient->SendProtoMessage(telnetProto);
	}

	void ConsoleComponent::OnReceive(std::shared_ptr<TelnetClientContext> clientContext, const std::string& message)
	{
		char cc = ' ';
		std::vector<std::string> splitStrings;
		std::shared_ptr<TelnetProto> telnetProto(new TelnetProto());
		google::protobuf::SplitStringUsing(message, &cc, &splitStrings);
		for(const std::string & str : splitStrings)
		{
			telnetProto->Add(str);
		}
		clientContext->StartRead();
		clientContext->SendProtoMessage(telnetProto);
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
			if(this->GetComponent<LocalRpcService>(name) != nullptr)
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
	void ConsoleComponent::OnClientError(std::shared_ptr<Tcp::TelnetClientContext> clientContext)
	{
		auto iter = this->mTelnetClients.find(clientContext);
		if(iter != this->mTelnetClients.end())
		{
			this->mTelnetClients.erase(iter);
			CONSOLE_LOG_ERROR("[" << clientContext->GetAddress() << "] disconnect");
		}
	}
}