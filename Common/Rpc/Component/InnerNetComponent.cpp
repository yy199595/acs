
#include"InnerNetComponent.h"
#include"XCode/XCode.h"
#include"Entity/Actor/App.h"
#include"Util/String/StringHelper.h"
#include"Network/Tcp/SocketProxy.h"
#include"DispatchComponent.h"
#include"Util/File/FileHelper.h"
#include"Server/Component/ThreadComponent.h"
#include"google/protobuf/util/json_util.h"
#include"Core/Singleton/Singleton.h"
#include"XCode/XCode.h"
namespace Tendo
{
    InnerNetComponent::InnerNetComponent()
		: ISender(Msg::Net::Tcp)
    {
        this->mSumCount = 0;
        this->mMaxHandlerCount = 0;
		this->mDisComponent = nullptr;
        this->mNetComponent = nullptr;
    }

    bool InnerNetComponent::LateAwake()
    {
        std::string path;
        rapidjson::Document document;
        this->mMaxHandlerCount = 2000;
        const ServerConfig *config = ServerConfig::Inst();
        LOG_CHECK_RET_FALSE(config->GetPath("user", path))
        config->GetMember("message", "inner", this->mMaxHandlerCount);
        LOG_CHECK_RET_FALSE(Helper::File::ReadJsonFile(path, document))
        auto iter = document.MemberBegin();
        for (; iter != document.MemberEnd(); ++iter)
        {
            const std::string user(iter->name.GetString());
            const std::string passwd(iter->value.GetString());
            this->mUserMaps.emplace(user, passwd);
        }
        LOG_CHECK_RET_FALSE(this->mNetComponent = this->GetComponent<ThreadComponent>())
        LOG_CHECK_RET_FALSE(this->mDisComponent = this->GetComponent<DispatchComponent>())
		if(config->GetLocation("rpc", this->mLocation))
		{
			LOG_CHECK_RET_FALSE(this->StartListen("rpc"));
		}
		return true;
    }

    void InnerNetComponent::OnConnectSuccessful(const std::string &address)
    {

    }

    void InnerNetComponent::OnMessage(std::shared_ptr<Msg::Packet> message)
    {
        this->mSumCount++;
        const int type = message->GetType();
		message->SetNet(Msg::Net::Tcp);
        switch (type)
        {
            case Msg::Type::Auth:
                this->OnAuth(message);
                break;
            case Msg::Type::Logout:
                this->StartClose(message->From());
                break;
            case Msg::Type::Request:
                this->mWaitMessages.push(message);
                break;
            case Msg::Type::Forward:
            case Msg::Type::Response:
            case Msg::Type::Broadcast:
                this->mDisComponent->OnMessage(message);
                break;
            default:
            LOG_FATAL("unknown message type : " << message->GetType())
                break;
        }
    }

    void InnerNetComponent::OnSendFailure(const std::string &address, std::shared_ptr<Msg::Packet> message)
    {
        if (message->GetType() == Msg::Type::Request)
        {
            if (message->GetHead().Has("rpc"))
            {
                message->SetType(Msg::Type::Response);
                message->GetHead().Add("code", XCode::NetWorkError);
                this->mDisComponent->OnMessage(message);
            }
        }
    }

    bool InnerNetComponent::OnAuth(const std::shared_ptr<Msg::Packet> &message)
    {
        NodeInfo nodeInfo;
        const Msg::Head &head = message->GetHead();
        const std::string &address = message->From();

        LOG_CHECK_RET_FALSE(head.Get("name", nodeInfo.SrvName))
        LOG_CHECK_RET_FALSE(head.Get("user", nodeInfo.UserName))
        LOG_CHECK_RET_FALSE(head.Get("passwd", nodeInfo.PassWord))
        return true;
    }

    bool InnerNetComponent::IsAuth(const std::string &address)
    {
		auto iter1 = this->mLocalClients.find(address);
		if(iter1 != this->mLocalClients.end())
		{
			return true;
		}
        auto iter = this->mRemoteClients.find(address);
        return iter != this->mRemoteClients.end();
    }


    void InnerNetComponent::OnCloseSocket(const std::string &address, int code)
    {
        auto iter = this->mRemoteClients.find(address);
        if (iter != this->mRemoteClients.end())
        {
            this->mRemoteClients.erase(iter);
			// TODO
        }
    }

    void InnerNetComponent::OnListen(std::shared_ptr<Tcp::SocketProxy> socket)
    {
        const std::string &address = socket->GetAddress();

		std::shared_ptr<InnerNetTcpClient> tcpSession
			= std::make_shared<InnerNetTcpClient>(this, socket);

		tcpSession->StartReceive();
		this->mRemoteClients.emplace(address, tcpSession);
    }

    void InnerNetComponent::StartClose(const std::string &address)
    {
        auto iter = this->mRemoteClients.find(address);
        if (iter != this->mRemoteClients.end())
        {
            if (iter->second != nullptr)
            {
				iter->second->StartClose();
            }
        }
    }

    InnerNetTcpClient *InnerNetComponent::GetLocalClient(const std::string& address)
    {
		auto iter = this->mRemoteClients.find(address);
		if(iter != this->mRemoteClients.end())
		{
			return iter->second.get();
		}
		auto iter1 = this->mLocalClients.find(address);
		if(iter1 != this->mLocalClients.end())
		{
			return iter1->second.get();
		}
        std::string ip;
        unsigned short port = 0;
        if (!Helper::Str::SplitAddr(address, ip, port))
        {
            CONSOLE_LOG_ERROR("parse address error : [" << address << "]")
            return nullptr;
        }
		AuthInfo authInfo;
        const ServerConfig *config = ServerConfig::Inst();
        {
            authInfo.ServerName = config->Name();
            config->GetLocation("rpc", authInfo.RpcAddress);
            config->GetMember("user", "name", authInfo.UserName);
            config->GetMember("user", "passwd", authInfo.PassWord);
        }

		std::shared_ptr<Tcp::SocketProxy> socketProxy = this->mNetComponent->CreateSocket(ip, port);
        std::shared_ptr<InnerNetTcpClient> localClient = std::make_shared<InnerNetTcpClient>(this, socketProxy, authInfo);

        this->mLocalClients.emplace(address, localClient);
        return localClient.get();
    }

    InnerNetTcpClient *InnerNetComponent::GetRemoteClient(const std::string& address)
    {
        auto iter = this->mRemoteClients.find(address);
        if (iter == this->mRemoteClients.end())
        {
            return nullptr;
        }
        return iter->second.get();
    }

    int InnerNetComponent::Send(const std::string &addr, const std::shared_ptr<Msg::Packet> &message)
    {
        if (addr.empty() || addr == this->mLocation) //发送到本机
        {
            message->SetFrom(addr);
            this->OnMessage(message);
            return XCode::Successful;
        }
        InnerNetTcpClient *clientSession = nullptr;
        switch (message->GetType())
        {
            case Msg::Type::Response:
                clientSession = this->GetRemoteClient(addr);
                break;
            default:
                clientSession = this->GetLocalClient(addr);
                break;
        }
        if (clientSession == nullptr)
        {
            LOG_ERROR("not find rpc client : [" << addr << "]")
            return XCode::SendMessageFail;
        }
        clientSession->Send(message);
        return XCode::Successful;
    }

    void InnerNetComponent::OnRecord(Json::Writer &document)
    {
        document.Add("sum").Add(this->mSumCount);
        document.Add("auth").Add(this->mLocationMaps.size());
        document.Add("client").Add(this->mRemoteClients.size());
    }

    void InnerNetComponent::OnFrameUpdate(float t)
    {
        for (int index = 0; index < this->mMaxHandlerCount && !this->mWaitMessages.empty(); index++)
        {
            std::shared_ptr<Msg::Packet> message = this->mWaitMessages.front();
            {
                int code = this->mDisComponent->OnMessage(message);
                if (code != XCode::Successful && message->GetHead().Has("rpc"))
                {
                    message->Clear();
                    message->GetHead().Add("code", code);
                    message->SetType(Msg::Type::Response);
                    this->Send(message->From(), message);
                }
            }
            this->mWaitMessages.pop();
        }
    }

    size_t InnerNetComponent::GetConnectClients(std::vector<std::string> &list) const
    {
        size_t count = 0;
		list.reserve(this->mRemoteClients.size());
        auto iter = this->mRemoteClients.begin();
        for (; iter != this->mRemoteClients.end(); iter++)
		{
			count++;
			list.emplace_back(iter->first);
		}
        return count;
    }


    size_t InnerNetComponent::Broadcast(const std::shared_ptr<Msg::Packet> &message) const
    {
        size_t count = 0;
        auto iter = this->mRemoteClients.begin();
        for (; iter != this->mRemoteClients.end(); iter++)
		{
			count++;
			iter->second->Send(message->Clone());
		}
        return count;
    }

	void InnerNetComponent::OnDestroy()
    {
        this->StopListen();
        auto iter = this->mRemoteClients.begin();
        for (; iter != this->mRemoteClients.end(); iter++)
        {
            this->StartClose(iter->first);
        }
    }
}// namespace Sentry