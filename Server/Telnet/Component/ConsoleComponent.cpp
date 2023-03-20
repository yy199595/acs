#include"ConsoleComponent.h"

#include"Service/PhysicalService.h"
#include"Component/AsyncMgrComponent.h"
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
		this->mTaskComponent = this->GetComponent<AsyncMgrComponent>();
		return true;
	}

    bool ConsoleComponent::Start()
    {
        return this->StartListen("console");
    }

	void ConsoleComponent::OnListen(std::shared_ptr<SocketProxy> socket)
	{
		std::shared_ptr<TelnetClientContext> telnetClient =
			std::make_shared<TelnetClientContext>(socket, this);

		const std::string & address = socket->GetAddress();
		std::shared_ptr<TelnetProto> telnetProto(new TelnetProto());
		telnetProto->Add("welcome connect sertry server");


		telnetClient->StartRead();
		this->mTelnetClients.emplace(address, telnetClient);
		telnetClient->SendProtoMessage(telnetProto);
	}

	void ConsoleComponent::OnReceive(const std::string & address, const std::string& message)
	{
		std::shared_ptr<Tcp::TelnetClientContext> telnetClientContext = this->GetClient(address);
		if(telnetClientContext != nullptr)
		{
			std::vector<std::string> splitStrings;
			std::shared_ptr<TelnetProto> telnetProto(new TelnetProto());
			google::protobuf::SplitStringUsing(message, "\r\n", &splitStrings);
            if(!this->Invoke(splitStrings, telnetProto))
            {
                telnetProto->Add("===== CMD ERR =====");
            }
            else
            {
                telnetProto->Add("===== CMD OK =====");
            }

			telnetClientContext->StartRead();
			telnetClientContext->SendProtoMessage(telnetProto);
		}
	}

    bool ConsoleComponent::Invoke(std::vector<std::string> &request, std::shared_ptr<TelnetProto> response)
    {
        if(request.empty())
        {
            response->Add("request empty");
            return false;
        }
        const std::string & cmd = request[0];
        auto iter = this->mFunctionMap.find(cmd);
        if(iter == this->mFunctionMap.end())
        {
            response->Add("unknow cmd " + cmd);
            return false;
        }
        std::vector<Component *> components;
        this->mApp->GetComponents(components);
        for(Component * component : components)
        {
           IHotfix *hotfix = component->Cast<IHotfix>();
           if(hotfix != nullptr)
           {
               hotfix->OnHotFix();
               response->Add(component->GetName());
           }
        }
        return true;
    }

	std::shared_ptr<Tcp::TelnetClientContext> ConsoleComponent::GetClient(const std::string& address)
	{
		auto iter = this->mTelnetClients.find(address);
		return iter != this->mTelnetClients.end() ? iter->second : nullptr;
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
		return true;
	}

	bool ConsoleComponent::Close(const std::string& parameter, std::vector<std::string>& response)
	{
        this->mApp->Stop();
		return true;
	}

	bool ConsoleComponent::Services(const std::string& parameter, std::vector<std::string>& response)
	{
		std::vector<std::string> components;
		this->mApp->GetComponents(components);
		for(const std::string & name : components)
		{
			if(this->GetComponent<PhysicalService>(name) != nullptr)
			{
				response.emplace_back(name);
			}
		}
		return true;
	}

	bool ConsoleComponent::Offset(const string& parameter, std::vector<string>& response)
	{
		long long value = std::stoll(parameter);
		Helper::Time::SetScaleTotalTime(value);
		LOG_WARN("now time = " << Helper::Time::GetDateString());
		return true;
	}
	void ConsoleComponent::OnClientError(const std::string & address)
	{
		auto iter = this->mTelnetClients.find(address);
		if(iter != this->mTelnetClients.end())
		{
			this->mTelnetClients.erase(iter);
			CONSOLE_LOG_ERROR("[" << address << "] disconnect");
		}
	}
}