
#include"InnerNetComponent.h"
#include"String/StringHelper.h"
#include"Tcp/SocketProxy.h"
#include"InnerNetMessageComponent.h"
#include"File/FileHelper.h"
#include"Config/CodeConfig.h"
#include"Service/Node.h"
#include"Component/TranComponent.h"
#include"Component/OuterNetComponent.h"
#include"Component/ThreadComponent.h"
#include"google/protobuf/util/json_util.h"
namespace Sentry
{
	bool InnerNetComponent::Awake()
	{
		this->mSumCount = 0;
		this->mWaitCount = 0;
		this->mNetComponent = nullptr;
        this->mTranComponent = nullptr;
		this->mMessageComponent = nullptr;
		return true;
	}

	bool InnerNetComponent::LateAwake()
	{
        std::string path;
        rapidjson::Document document;
        const ServerConfig * config = ServerConfig::Inst();
        LOG_CHECK_RET_FALSE(config->GetPath("user", path));
        LOG_CHECK_RET_FALSE(config->GetLocation("rpc", this->mLocation));
        LOG_CHECK_RET_FALSE(Helper::File::ReadJsonFile(path, document));
        auto iter = document.MemberBegin();
        for(; iter != document.MemberEnd(); iter++)
        {
            const std::string user(iter->name.GetString());
            const std::string passwd(iter->value.GetString());
            this->mUserMaps.emplace(user, passwd);
        }
        this->mTranComponent = this->GetComponent<TranComponent>();
        this->mOuterComponent = this->GetComponent<OuterNetComponent>();
        LOG_CHECK_RET_FALSE(this->mNetComponent = this->GetComponent<ThreadComponent>());
        LOG_CHECK_RET_FALSE(this->mMessageComponent = this->GetComponent<InnerNetMessageComponent>());
		return this->StartListen("rpc");
	}

	void InnerNetComponent::OnConnectSuccessful(const std::string& address)
	{

	}

    void InnerNetComponent::OnMessage(std::shared_ptr<Rpc::Packet> message)
    {
        const std::string& address = message->From();
		if(address != this->mLocation)
		{
			if (message->GetType() != (int)Tcp::Type::Auth && !this->IsAuth(address))
			{
				this->StartClose(address);
				CONSOLE_LOG_ERROR("close " << address << " not auth");
				return;
			}
		}
        switch ((Tcp::Type) message->GetType())
        {
            case Tcp::Type::Auth:
                if (!this->OnAuth(address, message))
                {
                    this->StartClose(address);
                    CONSOLE_LOG_ERROR("auth error " << address);
                }
                break;
            case Tcp::Type::Ping:
                this->OnPing(address, message);
                break;
            case Tcp::Type::Request:
                this->OnRequest(address, message);
                break;
            case Tcp::Type::Forward:
                this->OnForward(message);
                break;
            case Tcp::Type::Broadcast:
                this->OnBroadcast(message);
                break;
            case Tcp::Type::Response:
                this->OnResponse(address, message);
                break;
            default:
            {
                LOG_ERROR(address << " unknown message type : " << message->GetType());
            }
        }
    }

    void InnerNetComponent::OnSendFailure(const std::string& address, std::shared_ptr<Rpc::Packet> message)
    {
        if (message->GetType() == (int)Tcp::Type::Request)
        {
            if (message->GetHead().Has("rpc"))
            {
                message->SetType(Tcp::Type::Request);
                message->GetHead().Add("code", XCode::NetWorkError);
                this->OnResponse(address, message);
                return;
            }
        }
    }

    bool InnerNetComponent::OnPing(const std::string& address, std::shared_ptr<Rpc::Packet> message)
    {
        message->SetContent("pong");
        message->SetType(Tcp::Type::Response);
		message->GetHead().Add("code", XCode::Successful);
		return this->Send(address, message);
    }

    bool InnerNetComponent::OnAuth(const std::string & address, std::shared_ptr<Rpc::Packet> message)
    {
        const Rpc::Head &head = message->GetHead();
        std::unique_ptr<ServiceNodeInfo> serverNode(new ServiceNodeInfo());
        {
            LOG_CHECK_RET_FALSE(head.Get("name", serverNode->SrvName));
            LOG_CHECK_RET_FALSE(head.Get("user", serverNode->UserName));
            LOG_CHECK_RET_FALSE(head.Get("rpc", serverNode->RpcAddress));
            LOG_CHECK_RET_FALSE(head.Get("passwd", serverNode->PassWord));
            if (serverNode->RpcAddress.empty())
            {
                return false;
            }
            if (!this->mUserMaps.empty())
            {
                auto iter = this->mUserMaps.find(serverNode->UserName);
                if (iter == this->mUserMaps.end() || iter->second != serverNode->PassWord)
                {
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

	bool InnerNetComponent::OnRequest(const std::string& address, std::shared_ptr<Rpc::Packet> message)
	{
        Rpc::Head& head = message->GetHead();
        LOG_CHECK_RET_FALSE(head.Has("func"));
        if (this->mTranComponent != nullptr && head.Has("tran"))
        {
            head.Add("from", address);
            this->mTranComponent->OnRequest(message);
            return true;
        }
        LOG_CHECK_RET_FALSE(head.Add("address", address));
        int code = this->mMessageComponent->OnRequest(message);
        if(code != XCode::Successful)
        {
            std::string func;
            head.Get("func", func);
#ifndef __DEBUG__
            CONSOLE_LOG_ERROR("call " << func << 
                " code = " << CodeConfig::Inst()->GetDesc(code));
#endif
            if (!head.Has("rpc"))
            {
                return false;
            }
            head.Add("code", code);
            if (head.Has("address"))
            {
                head.Remove("id");
                head.Remove("address");
#ifndef __DEBUG__
                head.Remove("func");
#endif
                message->SetType(Tcp::Type::Response);
            }
            this->Send(address, message);
            return false;
        }
        return true;
	}

    bool InnerNetComponent::OnForward(std::shared_ptr<Rpc::Packet> message)
    {
        long long userId = 0;
        LOG_CHECK_RET_FALSE(this->mOuterComponent != nullptr);
        LOG_CHECK_RET_FALSE(message->GetHead().Has("func"));
        LOG_CHECK_RET_FALSE(message->GetHead().Get("id", userId));
        {
            message->GetHead().Remove("id");
            message->SetType(Tcp::Type::Request);
            message->GetHead().Remove("address");
        }
        return this->mOuterComponent->SendData(userId, message);
    }

    bool InnerNetComponent::OnBroadcast(std::shared_ptr<Rpc::Packet> message)
    {
        LOG_CHECK_RET_FALSE(this->mOuterComponent != nullptr);
        LOG_CHECK_RET_FALSE(message->GetHead().Has("func"));
        {
            message->SetType(Tcp::Type::Broadcast);
            message->GetHead().Remove("address");
        }
        return this->mOuterComponent->SendData(message);
    }

	bool InnerNetComponent::OnResponse(const std::string& address, std::shared_ptr<Rpc::Packet> message)
	{
        std::string target;
        Rpc::Head &head = message->GetHead();
        // 网关转发过来的消息 必须带client字段
        if(this->mOuterComponent != nullptr && head.Get("client", target))
        {
            head.Remove("client");
            message->SetFrom(target);
			this->mOuterComponent->OnMessage(message);
            return true;
        }
		if(head.Get("resp", target))
		{
			head.Remove("resp");
			this->Send(target, message);
			return true;
		}
#ifdef __DEBUG__
        int code = 0;
        LOG_CHECK_RET_FALSE(head.Get("code", code));
        if(code != (int)XCode::Successful)
        {
            std::string error, func;
            if(head.Get("func", func) && head.Get("error", error))
            {
                CONSOLE_LOG_ERROR("call " << func << " [" << address << "]" << "code = " << error);
            }
        }
#endif
        long long rpcId = 0;
        LOG_CHECK_RET_FALSE(head.Get("rpc", rpcId));
		this->mMessageComponent->OnResponse(rpcId, message);
		return true;
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
        if(!Helper::Str::SplitAddress(address, ip, port))
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


	bool InnerNetComponent::Send(const std::string & address, std::shared_ptr<Rpc::Packet> message)
	{
		if(address == this->mLocation) //发送到本机
		{
            message->SetFrom(address);
			this->OnMessage(message);
			return true;
		}
        InnerNetClient * clientSession = this->GetOrCreateSession(address);
		if (message == nullptr || clientSession == nullptr)
		{
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
}// namespace Sentry