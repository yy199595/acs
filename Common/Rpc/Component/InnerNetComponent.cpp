
#include"InnerNetComponent.h"
#include"String/StringHelper.h"
#include"Tcp/SocketProxy.h"
#include"InnerNetMessageComponent.h"
#include"File/FileHelper.h"
#include"Config/CodeConfig.h"
#include"Service/InnerService.h"
#include"Component/OuterNetComponent.h"
#include"Component/NetThreadComponent.h"
#include"google/protobuf/util/json_util.h"
namespace Sentry
{
	bool InnerNetComponent::Awake()
	{
		this->mNetComponent = nullptr;
		this->mMessageComponent = nullptr;
		return true;
	}

	bool InnerNetComponent::LateAwake()
	{
        std::string path;
        rapidjson::Document document;
        const ServerConfig * config = ServerConfig::Inst();
        LOG_CHECK_RET_FALSE(config->GetPath("user", path));
        LOG_CHECK_RET_FALSE(Helper::File::ReadJsonFile(path, document));
        auto iter = document.MemberBegin();
        for(; iter != document.MemberEnd(); iter++)
        {
            const std::string user(iter->name.GetString());
            const std::string passwd(iter->value.GetString());
            this->mUserMaps.emplace(user, passwd);
        }
        this->mOuterComponent = this->GetComponent<OuterNetComponent>();
        LOG_CHECK_RET_FALSE(config->GetMember("user", "name", this->mUserName));
        LOG_CHECK_RET_FALSE(config->GetMember("user", "passwd", this->mPassword));
        LOG_CHECK_RET_FALSE(this->mNetComponent = this->GetComponent<NetThreadComponent>());
        LOG_CHECK_RET_FALSE(this->mMessageComponent = this->GetComponent<InnerNetMessageComponent>());
		return this->StartListen("rpc");
	}

	void InnerNetComponent::OnConnectSuccessful(const std::string& address)
	{

	}

    void InnerNetComponent::OnMessage(const std::string &address, std::shared_ptr<Rpc::Packet> message)
    {
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
                LOG_ERROR(address << " unknow message type : " << message->GetType());
            }
        }
    }

    bool InnerNetComponent::OnAuth(const std::string & address, std::shared_ptr<Rpc::Packet> message)
    {
        const Rpc::Head &head = message->GetHead();
        std::unique_ptr<ServiceNodeInfo> serverNode(new ServiceNodeInfo());

        head.Get("rpc", serverNode->LocationRpc);
        head.Get("http", serverNode->LocationHttp);
        LOG_CHECK_RET_FALSE(head.Get("name", serverNode->SrvName));
        LOG_CHECK_RET_FALSE(head.Get("user", serverNode->UserName));
        LOG_CHECK_RET_FALSE(head.Get("passwd", serverNode->PassWord));
        if(serverNode->LocationRpc.empty() && serverNode->LocationHttp.empty())
        {
            return false;
        }
        if(!this->mUserMaps.empty())
        {
            auto iter = this->mUserMaps.find(serverNode->UserName);
            if (iter == this->mUserMaps.end() || iter->second != serverNode->PassWord)
            {
                CONSOLE_LOG_ERROR(address << " auth failure");
                return false;
            }
        }
        this->mLocationMaps.emplace(address, std::move(serverNode));
        return true;
    }

    const ServiceNodeInfo *InnerNetComponent::GetSeverInfo(const std::string &address)
    {
        auto iter = this->mLocationMaps.find(address);
        return iter != this->mLocationMaps.end() ? iter->second.get() : nullptr;
    }

    bool InnerNetComponent::IsAuth(const std::string &address)
    {
        auto iter = this->mRpcClientMap.find(address);
        return iter != this->mRpcClientMap.end();
    }

	bool InnerNetComponent::OnRequest(const std::string& address, std::shared_ptr<Rpc::Packet> message)
	{
        message->GetHead().Add("address", address);
        LOG_CHECK_RET_FALSE(message->GetHead().Has("func"));
        XCode code = this->mMessageComponent->OnRequest(message);
        if(code != XCode::Successful)
        {
            std::string func;
            message->GetHead().Get("func", func);
            CONSOLE_LOG_ERROR("call " << func << " code = " << CodeConfig::Inst()->GetDesc(code));
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
        std::string targer;
        const Rpc::Head &head = message->GetHead();
        if(this->mOuterComponent != nullptr && head.Get("client", targer))
        {
            message->GetHead().Remove("client");
            this->mOuterComponent->SendData(targer, message);
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
                CONSOLE_LOG_ERROR("call [" << address << "] func = {" << func << "}  code = " << error);
            }
        }
#endif
        long long rpcId = 0;
        LOG_CHECK_RET_FALSE(head.Get("rpc", rpcId));
		this->mMessageComponent->OnResponse(rpcId, message);
		return true;
	}

	void InnerNetComponent::OnCloseSocket(const std::string & address, XCode code)
	{
		auto iter = this->mRpcClientMap.find(address);
		if (iter != this->mRpcClientMap.end())
		{
			this->mRpcClientMap.erase(iter);
			LOG_WARN("close server address : " << address);
		}
	}

	bool InnerNetComponent::OnListen(std::shared_ptr<SocketProxy> socket)
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
            return true;
		}
        return false;
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
        if(!Helper::String::ParseIpAddress(address, ip, port))
        {
            CONSOLE_LOG_ERROR("parse address error : [" << address << "]");
            return nullptr;
        }
		std::shared_ptr<SocketProxy> socketProxy = this->mNetComponent->CreateSocket();
        if(socketProxy == nullptr)
        {
            return nullptr;
        }
        socketProxy->Init(ip, port);
		std::shared_ptr<InnerNetClient> newClient =
            std::make_shared<InnerNetClient>(this, socketProxy);

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
			this->OnMessage(address, message);
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
}// namespace Sentry