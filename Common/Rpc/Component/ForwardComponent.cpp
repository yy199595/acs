//
// Created by zmhy0073 on 2022/10/14.
//

#include "ForwardComponent.h"

namespace Sentry
{
    bool ForwardComponent::LateAwake()
    {
        return this->StartListen("forward");
    }

    void ForwardComponent::StartClose(const std::string &address)
    {
        InnerNetClient * netClient = this->GetClient(address);
        if(netClient != nullptr)
        {
            netClient->StartClose();
        }
    }

    InnerNetClient *ForwardComponent::GetClient(const std::string &address)
    {
        auto iter = this->mInnerClients.find(address);
        return iter != this->mInnerClients.end() ? iter->second.get() : nullptr;
    }


    void ForwardComponent::OnCloseSocket(const std::string &address, XCode code)
    {
        auto iter = this->mInnerClients.find(address);
        if(iter != this->mInnerClients.end())
        {
            this->mInnerClients.erase(iter);
        }
    }

    void ForwardComponent::OnMessage(const std::string &address, std::shared_ptr<Rpc::Data> message)
    {
        switch ((Tcp::Type) message->GetType())
        {
            case Tcp::Type::Auth:

                break;
            case Tcp::Type::Request:
            {
                XCode code = this->OnRequest(address, message);
                if (code != XCode::Successful)
                {
                    InnerNetClient *netClient = this->GetClient(address);
                    if (netClient != nullptr)
                    {
                        message->SetType(Tcp::Type::Response);
                        message->GetHead().Add("code", code);
                        netClient->SendData(message);
                    }
                }
            }
                break;
            case Tcp::Type::Broadcast:
            {
                auto iter = this->mInnerClients.begin();
                for(; iter != this->mInnerClients.end(); iter++)
                {
                    iter->second->SendData(message);
                }
            }
                break;
            case Tcp::Type::Response:

                break;
        }
    }

    XCode ForwardComponent::OnRequest(const std::string & address, std::shared_ptr<Rpc::Data> message)
    {
        std::string target;
        if(!message->GetHead().Get("to", target))
        {
            return XCode::CallArgsError;
        }
        message->GetHead().Add("resp", address);
        return XCode::Successful;
    }

    bool ForwardComponent::IsAuth(const std::string &address) const
    {
        auto iter = this->mAuthClients.find(address);
        return iter != this->mAuthClients.end();
    }

    bool ForwardComponent::OnListen(std::shared_ptr<SocketProxy> socket)
    {
        std::shared_ptr<InnerNetClient> netClient =
            std::make_shared<InnerNetClient>(this, socket);
        if(netClient != nullptr)
        {
            netClient->StartReceive();
            const std::string & address = socket->GetAddress();
            this->mInnerClients.emplace(address, netClient);
        }
        return true;
    }
}