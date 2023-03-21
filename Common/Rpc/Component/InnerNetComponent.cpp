
#include"InnerNetComponent.h"
#include"String/StringHelper.h"
#include"Tcp/SocketProxy.h"
#include"InnerNetMessageComponent.h"
#include"File/FileHelper.h"
#include"Config/CodeConfig.h"
#include"Component/ThreadComponent.h"
#include"google/protobuf/util/json_util.h"
namespace Sentry
{
	bool InnerNetComponent::Awake()
	{
		this->mSumCount = 0;
		this->mWaitCount = 0;
		this->mNetComponent = nullptr;
		this->mMessageComponent = nullptr;
		return true;
	}

	bool InnerNetComponent::LateAwake()
	{
        std::string path;
        rapidjson::Document document;
		this->mMaxHandlerCount = 200;
        const ServerConfig * config = ServerConfig::Inst();
        LOG_CHECK_RET_FALSE(config->GetPath("user", path));
		config->GetMember("message", "inner", this->mMaxHandlerCount);
		LOG_CHECK_RET_FALSE(config->GetLocation("rpc", this->mLocation));
        LOG_CHECK_RET_FALSE(Helper::File::ReadJsonFile(path, document));
        auto iter = document.MemberBegin();
        for(; iter != document.MemberEnd(); iter++)
        {
            const std::string user(iter->name.GetString());
            const std::string passwd(iter->value.GetString());
            this->mUserMaps.emplace(user, passwd);
        }  
        LOG_CHECK_RET_FALSE(this->mNetComponent = this->GetComponent<ThreadComponent>());
        LOG_CHECK_RET_FALSE(this->mMessageComponent = this->GetComponent<InnerNetMessageComponent>());
		return this->StartListen("rpc");
	}

	void InnerNetComponent::OnConnectSuccessful(const std::string& address)
	{

	}

    void InnerNetComponent::OnMessage(std::shared_ptr<Rpc::Packet> message)
	{
		int type = message->GetType();
		const std::string& address = message->From();
		if (type != Tcp::Type::Auth && address != this->mLocation)
		{
			if (!this->IsAuth(address))
			{
				this->StartClose(address);
				CONSOLE_LOG_ERROR("close " << address << " not auth");
				return;
			}
		}
		switch (type)
		{
			case Tcp::Type::Auth:
				this->OnAuth(message);
				break;
			case Tcp::Type::Logout:
				this->StartClose(address);
				break;
			case Tcp::Type::Request:
				this->mWaitMessages.push(message);
				break;
			case Tcp::Type::Forward:
			case Tcp::Type::Response:
			case Tcp::Type::Broadcast:
				this->mMessageComponent->OnMessage(message);
				break;
			default:
			LOG_FATAL("unknown message type : " << message->GetType());
				break;
		}
	}

    void InnerNetComponent::OnSendFailure(const std::string& address, std::shared_ptr<Rpc::Packet> message)
    {
        if (message->GetType() == (int)Tcp::Type::Request)
        {
            if (message->GetHead().Has("rpc"))
            {
                message->SetType(Tcp::Type::Response);
                message->GetHead().Add("code", XCode::NetWorkError);
                this->mMessageComponent->OnMessage(std::move(message));
                return;
            }
        }
    }

    bool InnerNetComponent::OnAuth(std::shared_ptr<Rpc::Packet> message)
    {
        const Rpc::Head &head = message->GetHead();
        const std::string& address = message->From();
        std::unique_ptr<ServiceNodeInfo> serverNode(new ServiceNodeInfo());
        {
            LOG_CHECK_RET_FALSE(head.Get("name", serverNode->SrvName));
            LOG_CHECK_RET_FALSE(head.Get("user", serverNode->UserName));
            LOG_CHECK_RET_FALSE(head.Get("rpc", serverNode->RpcAddress));
            LOG_CHECK_RET_FALSE(head.Get("passwd", serverNode->PassWord));
            if (serverNode->RpcAddress.empty())
            {
				this->StartClose(address);
                return false;
            }
            if (!this->mUserMaps.empty())
            {
                auto iter = this->mUserMaps.find(serverNode->UserName);
                if (iter == this->mUserMaps.end() || iter->second != serverNode->PassWord)
                {
					this->StartClose(address);
                    CONSOLE_LOG_ERROR(address << " auth failure");
                    return false;
                }
            }
            serverNode->LocalAddress = address;			
        }
        this->mLocationMaps.emplace(address, std::move(serverNode));
        return true;
    }

    bool InnerNetComponent::IsAuth(const std::string &address)
    {
        auto iter = this->mRpcClientMap.find(address);
        return iter != this->mRpcClientMap.end();
    }


	void InnerNetComponent::OnCloseSocket(const std::string & address, int code)
	{
		auto iter = this->mRpcClientMap.find(address);
		if (iter != this->mRpcClientMap.end())
		{
			this->mRpcClientMap.erase(iter);
			LOG_WARN("close server address : " << address);
		}
	}

    void InnerNetComponent::OnListen(std::shared_ptr<SocketProxy> socket)
	{
		const std::string& address = socket->GetAddress();
		auto iter = this->mRpcClientMap.find(address);
		if (iter == this->mRpcClientMap.end())
		{
			assert(!address.empty());
			std::shared_ptr<InnerNetClient> tcpSession
					= std::make_shared<InnerNetClient>(this, socket);

			tcpSession->StartReceive();
			this->mRpcClientMap.emplace(address, tcpSession);
		}
	}

	void InnerNetComponent::StartClose(const std::string & address)
	{
		auto iter = this->mRpcClientMap.find(address);
		if (iter != this->mRpcClientMap.end())
		{
			std::shared_ptr<InnerNetClient> innerNetClient = iter->second;
            if(innerNetClient != nullptr)
            {
                innerNetClient->StartClose();
            }
			this->mRpcClientMap.erase(iter);
		}
	}

	InnerNetClient * InnerNetComponent::GetOrCreateSession(const std::string& address)
	{
		InnerNetClient * localSession = this->GetSession(address);
		if (localSession != nullptr)
		{
			return localSession;
		}
		std::string ip;
		unsigned short port = 0;
        if(!Helper::Str::SplitAddr(address, ip, port))
        {
            CONSOLE_LOG_ERROR("parse address error : [" << address << "]");
            return nullptr;
        }
		std::shared_ptr<SocketProxy> socketProxy = this->mNetComponent->CreateSocket();
        if(socketProxy == nullptr)
        {
            return nullptr;
        }
		AuthInfo authInfo;
		const ServerConfig * config = ServerConfig::Inst();
		{
			authInfo.ServerName = config->Name();
			config->GetLocation("rpc", authInfo.RpcAddress);
			config->GetMember("user", "name", authInfo.UserName);
			config->GetMember("user", "passwd", authInfo.PassWord);
		}

        socketProxy->Init(ip, port);
		std::shared_ptr<InnerNetClient> newClient =
            std::make_shared<InnerNetClient>(this, socketProxy, authInfo);

		this->mRpcClientMap.emplace(socketProxy->GetAddress(), newClient);
		return newClient.get();
	}

	InnerNetClient * InnerNetComponent::GetSession(const std::string& address)
	{
		auto iter = this->mRpcClientMap.find(address);
		if (iter == this->mRpcClientMap.end())
		{		
			return nullptr;
		}
		return iter->second.get();
	}


    bool InnerNetComponent::Send(std::shared_ptr<Rpc::Packet> message)
    {
        message->SetFrom(this->mLocation);
        this->mMessageComponent->OnMessage(message);
        return true;
    }

    bool InnerNetComponent::Send(const std::string & address, std::shared_ptr<Rpc::Packet> message)
	{
		if(address == this->mLocation) //发送到本机
		{
            message->SetFrom(address);
            //this->mMessageComponent->OnMessage(message);
			Asio::Context & io = this->mApp->MainThread();
			io.post(std::bind(&InnerNetComponent::OnMessage, this, message));
			return true;
		}
		InnerNetClient* clientSession = nullptr;
		switch (message->GetType())
		{
		case Tcp::Type::Response:
			clientSession = this->GetSession(address);
			break;
		default:
			clientSession = this->GetOrCreateSession(address);
			break;
		}
		if (clientSession == nullptr)
		{
			LOG_ERROR("not find rpc client : [" << address << "]");
			return false;
		}
		clientSession->Send(message);
		return true;
	}

    void InnerNetComponent::OnRecord(Json::Writer &document)
    {
        //document.Add("auth").Add( this->mLocationMaps.size());
        //document.Add("client").Add(this->mRpcClientMap.size());
    }

	const ServiceNodeInfo *InnerNetComponent::GetSeverInfo(const std::string &address) const
	{
		auto iter = this->mLocationMaps.find(address);		
		return iter != this->mLocationMaps.end() ? iter->second.get() : nullptr;
	}

    void InnerNetComponent::GetServiceList(std::vector<std::string>& list) const
    {
        auto iter = this->mLocationMaps.begin();
        for (; iter != this->mLocationMaps.end(); iter++)
        {
            list.push_back(iter->second->LocalAddress);
        }
    }

	void InnerNetComponent::GetServiceList(std::vector<const ServiceNodeInfo *> &list) const
	{
		auto iter = this->mLocationMaps.begin();
		for(; iter != this->mLocationMaps.end(); iter++)
		{
			list.push_back(iter->second.get());
		}
	}
    void InnerNetComponent::GetServiceList(const std::string& name, std::vector<const ServiceNodeInfo*>& list) const
    {
        if (name.empty())
        {
            this->GetServiceList(list);
            return;
        }
        auto iter = this->mLocationMaps.begin();
        for (; iter != this->mLocationMaps.end(); iter++)
        {
            if (iter->second->SrvName == name)
            {
                list.push_back(iter->second.get());
            }
        }
    }

	void InnerNetComponent::OnFrameUpdate(float t)
	{
		for(int index = 0; index < this->mMaxHandlerCount && !this->mWaitMessages.empty(); index++)
		{
			std::shared_ptr<Rpc::Packet> message = this->mWaitMessages.front();
			{
				int code = this->mMessageComponent->OnMessage(message);
				if(code != XCode::Successful && message->GetHead().Has("rpc"))
				{
					message->Clear();
					message->GetHead().Add("code", code);
					message->SetType(Tcp::Type::Response);
					this->Send(message->From(), message);
				}
			}
			this->mWaitMessages.pop();
		}
	}

	size_t InnerNetComponent::GetConnectClients(std::vector<std::string>& list) const
	{
		size_t count = 0;
		auto iter = this->mRpcClientMap.begin();
		for(; iter != this->mRpcClientMap.end(); iter++)
		{
			if(!iter->second->IsClient()) //连接进来的
			{
				count++;
				list.emplace_back(iter->first);
			}
		}
		return count;
	}


	size_t InnerNetComponent::Broadcast(std::shared_ptr<Rpc::Packet> message) const
	{
		size_t count = 0;
		auto iter = this->mRpcClientMap.begin();
		for(; iter != this->mRpcClientMap.end(); iter++)
		{
			if(!iter->second->IsClient()) //连接进来的
			{
				count++;
				iter->second->Send(message->Clone());
			}
		}
		return count;
	}
}// namespace Sentry