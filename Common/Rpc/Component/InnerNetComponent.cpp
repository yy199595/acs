
#include"InnerNetComponent.h"
#include"Entity/App/App.h"
#include"Util/String/StringHelper.h"
#include"Network/Tcp/SocketProxy.h"
#include"InnerNetMessageComponent.h"
#include"Util/File/FileHelper.h"
#include"Server/Component/ThreadComponent.h"
#include"google/protobuf/util/json_util.h"
#include"Core/Singleton/Singleton.h"

namespace Tendo
{
    InnerNetComponent::InnerNetComponent()
    {
        this->mSumCount = 0;
        this->mMaxHandlerCount = 0;
        this->mNetComponent = nullptr;
        this->mMessageComponent = nullptr;
    }

    bool InnerNetComponent::LateAwake()
    {
        std::string path;
        rapidjson::Document document;
        this->mMaxHandlerCount = 2000;
        const ServerConfig *config = ServerConfig::Inst();
        LOG_CHECK_RET_FALSE(config->GetPath("user", path));
        config->GetMember("message", "inner", this->mMaxHandlerCount);
        LOG_CHECK_RET_FALSE(config->GetLocation("rpc", this->mLocation));
        LOG_CHECK_RET_FALSE(Helper::File::ReadJsonFile(path, document));
        auto iter = document.MemberBegin();
        for (; iter != document.MemberEnd(); iter++)
        {
            const std::string user(iter->name.GetString());
            const std::string passwd(iter->value.GetString());
            this->mUserMaps.emplace(user, passwd);
        }
        NodeInfo nodeInfo;
        nodeInfo.SrvName = ServerConfig::Inst()->Name();
        config->GetLocation("rpc", nodeInfo.RpcAddress);
        config->GetLocation("http", nodeInfo.HttpAddress);
        this->mLocationMaps.emplace(nodeInfo.RpcAddress, nodeInfo);
        LOG_CHECK_RET_FALSE(this->mNetComponent = this->GetComponent<ThreadComponent>());
        LOG_CHECK_RET_FALSE(this->mMessageComponent = this->GetComponent<InnerNetMessageComponent>());
        return this->StartListen("rpc");
    }

    void InnerNetComponent::OnConnectSuccessful(const std::string &address)
    {

    }

    void InnerNetComponent::OnMessage(std::shared_ptr<Rpc::Packet> message)
    {
        this->mSumCount++;
        int type = message->GetType();
        const std::string &address = message->From();
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

    void InnerNetComponent::OnSendFailure(const std::string &address, std::shared_ptr<Rpc::Packet> message)
    {
        if (message->GetType() == (int) Tcp::Type::Request)
        {
            if (message->GetHead().Has("rpc"))
            {
                message->SetType(Tcp::Type::Response);
                message->GetHead().Add("code", XCode::NetWorkError);
                this->mMessageComponent->OnMessage(message);
                return;
            }
        }
    }

    bool InnerNetComponent::OnAuth(const std::shared_ptr<Rpc::Packet> &message)
    {
        NodeInfo nodeInfo;
        const Rpc::Head &head = message->GetHead();
        const std::string &address = message->From();

        LOG_CHECK_RET_FALSE(head.Get("name", nodeInfo.SrvName));
        LOG_CHECK_RET_FALSE(head.Get("user", nodeInfo.UserName));
        LOG_CHECK_RET_FALSE(head.Get("rpc", nodeInfo.RpcAddress));
        LOG_CHECK_RET_FALSE(head.Get("passwd", nodeInfo.PassWord));
        if (nodeInfo.RpcAddress.empty())
        {
            this->StartClose(address);
            return false;
        }
        if (!this->mUserMaps.empty())
        {
            auto iter = this->mUserMaps.find(nodeInfo.UserName);
            if (iter == this->mUserMaps.end() || iter->second != nodeInfo.PassWord)
            {
                this->StartClose(address);
                CONSOLE_LOG_ERROR(address << " auth failure");
                return false;
            }
        }
        nodeInfo.LocalAddress = address;
        this->mLocationMaps.emplace(address, nodeInfo);
        return true;
    }

    bool InnerNetComponent::IsAuth(const std::string &address)
    {
        auto iter = this->mRpcClientMap.find(address);
        return iter != this->mRpcClientMap.end();
    }


    void InnerNetComponent::OnCloseSocket(const std::string &address, int code)
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
        const std::string &address = socket->GetAddress();
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

    void InnerNetComponent::StartClose(const std::string &address)
    {
        auto iter = this->mRpcClientMap.find(address);
        if (iter != this->mRpcClientMap.end())
        {
            std::shared_ptr<InnerNetClient> innerNetClient = iter->second;
            if (innerNetClient != nullptr)
            {
                innerNetClient->StartClose();
            }
            this->mRpcClientMap.erase(iter);
        }
    }

    InnerNetClient *InnerNetComponent::GetOrCreateSession(const std::string &address)
    {
        InnerNetClient *localSession = this->GetSession(address);
        if (localSession != nullptr)
        {
            return localSession;
        }
        std::string ip;
        unsigned short port = 0;
        if (!Helper::Str::SplitAddr(address, ip, port))
        {
            CONSOLE_LOG_ERROR("parse address error : [" << address << "]");
            return nullptr;
        }
        std::shared_ptr<SocketProxy> socketProxy = this->mNetComponent->CreateSocket();
        if (socketProxy == nullptr)
        {
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

        socketProxy->Init(ip, port);
        std::shared_ptr<InnerNetClient> newClient =
                std::make_shared<InnerNetClient>(this, socketProxy, authInfo);

        this->mRpcClientMap.emplace(socketProxy->GetAddress(), newClient);
        return newClient.get();
    }

    InnerNetClient *InnerNetComponent::GetSession(const std::string &address)
    {
        auto iter = this->mRpcClientMap.find(address);
        if (iter == this->mRpcClientMap.end())
        {
            return nullptr;
        }
        return iter->second.get();
    }


    bool InnerNetComponent::Send(const std::shared_ptr<Rpc::Packet> &message)
    {
        message->SetFrom(this->mLocation);
        this->mMessageComponent->OnMessage(message);
        return true;
    }

    bool InnerNetComponent::Send(const std::string &address, int code, const std::shared_ptr<Rpc::Packet> &message)
    {
        if (!message->GetHead().Has("rpc"))
        {
            return false; //不需要返回
        }
        message->GetHead().Remove("id");
        message->GetHead().Add("code", code);
#ifndef __DEBUG__
        message->GetHead().Remove("func");
#endif
        message->SetType(Tcp::Type::Response);
        this->Send(message->From(), message);
        return true;
    }

    bool InnerNetComponent::Send(const std::string &address, const std::shared_ptr<Rpc::Packet> &message)
    {
        if (address == this->mLocation) //发送到本机
        {
            message->SetFrom(address);
            //this->mMessageComponent->OnMessage(message);
            Asio::Context &io = this->mApp->MainThread();
            io.post(std::bind(&InnerNetComponent::OnMessage, this, message));
            return true;
        }
        InnerNetClient *clientSession = nullptr;
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
        document.Add("sum").Add(this->mSumCount);
        document.Add("wait").Add(this->mMessageComponent->WaitCount());
        document.Add("auth").Add(this->mLocationMaps.size());
        document.Add("client").Add(this->mRpcClientMap.size());
    }

    const NodeInfo *InnerNetComponent::GetNodeInfo(const std::string &address) const
    {
        auto iter = this->mLocationMaps.find(address);
        return iter != this->mLocationMaps.end() ? &iter->second : nullptr;
    }

    void InnerNetComponent::OnFrameUpdate(float t)
    {
        for (int index = 0; index < this->mMaxHandlerCount && !this->mWaitMessages.empty(); index++)
        {
            std::shared_ptr<Rpc::Packet> message = this->mWaitMessages.front();
            {
                int code = this->mMessageComponent->OnMessage(message);
                if (code != XCode::Successful && message->GetHead().Has("rpc"))
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

    size_t InnerNetComponent::GetConnectClients(std::vector<std::string> &list) const
    {
        size_t count = 0;
        auto iter = this->mRpcClientMap.begin();
        for (; iter != this->mRpcClientMap.end(); iter++)
        {
            if (!iter->second->IsClient()) //连接进来的
            {
                count++;
                list.emplace_back(iter->first);
            }
        }
        return count;
    }


    size_t InnerNetComponent::Broadcast(const std::shared_ptr<Rpc::Packet> &message) const
    {
        size_t count = 0;
        auto iter = this->mRpcClientMap.begin();
        for (; iter != this->mRpcClientMap.end(); iter++)
        {
            if (!iter->second->IsClient()) //连接进来的
            {
                count++;
                iter->second->Send(message->Clone());
            }
        }
        return count;
    }

    void InnerNetComponent::OnDestroy()
    {
        this->StopListen();
        auto iter = this->mRpcClientMap.begin();
        for (; iter != this->mRpcClientMap.end(); iter++)
        {
            this->StartClose(iter->first);
        }
    }

}// namespace Sentry