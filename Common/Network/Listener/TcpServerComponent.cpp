#include"TcpServerComponent.h"
#include"App/App.h"
#include<Util/StringHelper.h>
#include"Network/SocketProxy.h"
#include<Component/Scene/NetThreadComponent.h>
#include<Network/Listener/TcpServerListener.h>
namespace Sentry
{
	bool TcpServerComponent::LoadServerConfig()
	{
		const ServerConfig& config = App::Get()->GetConfig();
		const rapidjson::Value* jsonValue = config.GetJsonValue("listener");
		if (jsonValue == nullptr || !jsonValue->IsObject())
		{
			return false;
		}
		return true;
	}

	bool TcpServerComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->LoadServerConfig());
        std::vector<const ListenConfig *> listenerConfigs;
		this->GetConfig().GetListeners(listenerConfigs);
		for (const ListenConfig * listenConfig : listenerConfigs)
		{
            const std::string & name = listenConfig->Handler;
            TcpServerListener * tcpComponent = this->GetComponent<TcpServerListener>(name);
            if(tcpComponent == nullptr || !tcpComponent->Init(listenConfig))
            {
                return false;
            }
		}
		return true;
	}

    void TcpServerComponent::OnComplete()
    {
        std::vector<const ListenConfig *> listenerConfigs;
        this->GetConfig().GetListeners(listenerConfigs);
        for (const ListenConfig * listenConfig : listenerConfigs)
        {
            const std::string &name = listenConfig->Handler;
            TcpServerListener *tcpComponent = this->GetComponent<TcpServerListener>(name);
            if(tcpComponent->StartInComplete())
            {
                if(tcpComponent->StartListen())
                {
                    LOG_INFO(listenConfig->Name << " listen [" << listenConfig->Address << "] successful");
                    continue;
                }
                LOG_ERROR(listenConfig->Name << " listen [" << listenConfig->Address << "] failure");
            }
        }
    }

    void TcpServerComponent::OnAllServiceStart()
    {
        std::vector<const ListenConfig *> listenerConfigs;
        this->GetConfig().GetListeners(listenerConfigs);
        for (const ListenConfig * listenConfig : listenerConfigs)
        {
            const std::string &name = listenConfig->Handler;
            TcpServerListener *tcpComponent = this->GetComponent<TcpServerListener>(name);
            if(!tcpComponent->StartInComplete())
            {
                if(tcpComponent->StartListen())
                {
                    LOG_INFO(listenConfig->Name << " listen [" << listenConfig->Address << "] successful");
                    continue;
                }
                LOG_ERROR(listenConfig->Name << " listen [" << listenConfig->Address << "] failure");
            }
        }
    }

    bool TcpServerComponent::AddBlackList(const std::string &ip)
    {
        if(this->mBlackList.find(ip) == this->mBlackList.end())
        {
            this->mBlackList.insert(ip);
            return true;
        }
        return false;
    }

    bool TcpServerComponent::AddWhiteList(const std::string &ip)
    {
        if(this->mWhiteList.find(ip) == this->mWhiteList.end())
        {
            this->mWhiteList.insert(ip);
            return true;
        }
        return false;
    }

    bool TcpServerComponent::OnListenConnect(std::shared_ptr<SocketProxy> socket)
    {
        if(!this->mWhiteList.empty())
        {
            const std::string & ip = socket->GetIp();
            if(this->mWhiteList.find(ip) == this->mWhiteList.end()) //不在白名单
            {
                LOG_ERROR("close socket not in white list " << socket->GetIp());
                return false;
            }
        }
        if(!this->mBlackList.empty())
        {
            const std::string & ip = socket->GetIp();
            if(this->mBlackList.find(ip) != this->mBlackList.end()) //不在白名单
            {
                LOG_ERROR("close socket not in blackk list " << socket->GetIp());
                return false;
            }
        }
        return true;
    }
}
