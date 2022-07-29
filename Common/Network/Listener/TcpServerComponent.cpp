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
			if (this->GetComponent<ISocketListen>(listenConfig->Handler) == nullptr)
			{
				LOG_ERROR("not find socket handler " << listenConfig->Handler);
				return false;
			}

			if (listenConfig->Port != 0)
			{
				TcpServerListener * listener =
					new TcpServerListener(listenConfig);
				this->mListeners.emplace(listenConfig->Name ,listener);
                this->mListenConfigs.emplace(listenConfig->Name, listenConfig);
			}
		}
#ifndef ONLY_MAIN_THREAD
        this->mThreadComponent = this->GetComponent<NetThreadComponent>();
#endif
		return true;
	}

    TcpServerListener *TcpServerComponent::GetListener(const std::string &name)
    {
        auto iter = this->mListeners.find(name);
        return iter != this->mListeners.end() ? iter->second : nullptr;
    }

	void TcpServerComponent::OnAllServiceStart()
	{
        asio::io_service & io = this->GetApp()->GetThread();
        auto iter = this->mListenConfigs.begin();
        for(; iter != this->mListenConfigs.end(); iter++)
        {
            const ListenConfig * listenConfig = iter->second;
            TcpServerListener * tcpServerListener = this->GetListener(listenConfig->Name);
            if(tcpServerListener == nullptr)
            {
                LOG_FATAL("not find listener " << listenConfig->Name);
                continue;
            }
            if(!tcpServerListener->StartListen(io, this))
            {
                LOG_FATAL(listenConfig->Name << " listen " << listenConfig->Ip << ":" << listenConfig->Port << " failure");
                continue;
            }
            LOG_DEBUG(listenConfig->Name << " listen " << listenConfig->Ip << ":" << listenConfig->Port << " successful");
        }
	}

    void TcpServerComponent::DeleteSocket(std::shared_ptr<SocketProxy> socket)
    {
        if(this->mSocketQueue.size() < 1000)
        {
            this->mSocketQueue.push(socket);
        }
    }

    std::shared_ptr<SocketProxy> TcpServerComponent::CreateSocket(const std::string &ip, unsigned short port)
    {
        std::shared_ptr<SocketProxy> socket = this->CreateSocket();
        socket->Init(ip, port);
        return socket;
    }

    std::shared_ptr<SocketProxy> TcpServerComponent::CreateSocket()
    {
        std::shared_ptr<SocketProxy> socket;
        assert(this->GetApp()->IsMainThread());
        if(!this->mSocketQueue.empty())
        {
            socket = this->mSocketQueue.front();
            this->mSocketQueue.pop();
        }
        else
        {
#ifdef ONLY_MAIN_THREAD
            asio::io_service & io = App::Get()->GetThread();
#else
            asio::io_service &io = this->mThreadComponent->AllocateNetThread();
#endif
            socket = std::make_shared<SocketProxy>(io);
        }
        return socket;
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

    bool TcpServerComponent::OnListenConnect(const std::string &name, std::shared_ptr<SocketProxy> socket)
    {
        auto iter = this->mListenConfigs.find(name);
        if(iter == this->mListenConfigs.end())
        {
            return false;
        }
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
        const std::string & component = iter->second->Handler;
        ISocketListen * socketComponent = this->GetComponent<ISocketListen>(component);
        if(socketComponent == nullptr)
        {
            return false;
        }
        socketComponent->OnListen(socket);
        LOG_DEBUG(name << " listen new socket " << socket->GetAddress());
        return true;
    }
}
