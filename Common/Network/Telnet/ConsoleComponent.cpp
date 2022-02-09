#include"ConsoleComponent.h"
#include"Coroutine/TaskComponent.h"
#include"Scene/OperatorComponent.h"
#include"Service/RpcService.h"
#include"Service/LocalService.h"
#define BIND_FUNC(name, func) this->mFunctionMap.emplace(name, std::bind(&func, this, args1, args2));

namespace Sentry
{
	bool ConsoleComponent::Awake()
	{
        BIND_FUNC("help", ConsoleComponent::Help);
        BIND_FUNC("close", ConsoleComponent::Close);
        BIND_FUNC("hotfix", ConsoleComponent::Hotfix);
        BIND_FUNC("start", ConsoleComponent::Start);
        BIND_FUNC("service", ConsoleComponent::Services);
        return true;
	}

    bool ConsoleComponent::LateAwake()
    {
        this->mTaskComponent = this->GetComponent<TaskComponent>();
        return true;
    }

    void ConsoleComponent::OnListen(std::shared_ptr<SocketProxy> socket)
    {
        std::shared_ptr<TelnetClient> telnetClient(new TelnetClient(socket));
        this->mTaskComponent->Start(&ConsoleComponent::HandleConsoleClient, this, telnetClient);
    }

    void ConsoleComponent::HandleConsoleClient(std::shared_ptr<TelnetClient> telnetClient)
    {
        telnetClient->Response("welcome to sentry server");
        while(telnetClient->IsOpen())
        {
            std::shared_ptr<TelnetContent> commandContent = telnetClient->ReadCommand();
            LOG_WARN(commandContent->Command, "  ", commandContent->Paramater);
            if(commandContent->IsOk)
            {
                std::stringstream stringstream1;
                auto iter = this->mFunctionMap.find(commandContent->Command);
                if(iter == this->mFunctionMap.end())
                {
                    stringstream1 << "\r\n" << "<CMD NOT>";
                }
                else
                {
                    std::vector<std::string> response;
                    ConsoleFunction  function = iter->second;
                    if(function(commandContent->Paramater, response))
                    {
                        for(const std::string & str : response)
                        {
                            stringstream1 << "\r\n" << str;
                        }
                        stringstream1 << "\r\n<CMD OK>";
                    }
                    else
                    {
                        stringstream1 << "\r\n<CMD ERR>";
                    }
                }
                std::string res = stringstream1.str();
                telnetClient->Response(  stringstream1.str());
            }
        }
        LOG_ERROR("[console ]",telnetClient->GetAddress(), " disconnected");
    }

    bool ConsoleComponent::Help(const std::string &paramater, std::vector<std::string> &response)
    {
        auto iter = this->mFunctionMap.begin();
        for(; iter != this->mFunctionMap.end(); iter++)
        {
            response.emplace_back(iter->first);
        }
        return true;
    }

    bool ConsoleComponent::Start(const std::string &parameter, std::vector<std::string> &response)
    {
        LocalService * localService = this->GetComponent<LocalService>();
        if(localService == nullptr)
        {
            return false;
        }
        return localService->AddComponent(parameter);
    }

    bool ConsoleComponent::Close(const std::string &parameter, std::vector<std::string> &response)
    {
        App::Get().Stop(ExitCode::Exit);
        return true;
    }

    bool ConsoleComponent::Services(const std::string &parameter, std::vector<std::string> &response)
    {
        std::vector<Component *> components;
        this->GetComponents(components);
        RpcService * serviceComponent = nullptr;
        for(Component * component : components)
        {
            if(serviceComponent = dynamic_cast<RpcService*>(component))
            {
                response.emplace_back(serviceComponent->GetName());
            }
        }
        return true;
    }

    bool ConsoleComponent::Hotfix(const std::string &parameter, std::vector<std::string> &response)
    {
        auto operComponent = this->GetComponent<OperatorComponent>();
        operComponent->StartHotfix();
        return true;
    }
}