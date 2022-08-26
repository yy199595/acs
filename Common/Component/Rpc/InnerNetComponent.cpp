
#include"InnerNetComponent.h"
#include"App/App.h"
#include"Util/StringHelper.h"
#include"Network/SocketProxy.h"
#include"Component/Rpc/InnerNetMessageComponent.h"
#include"Global/ServiceConfig.h"
#include"Component/Scene/NetThreadComponent.h"

#include"Async/RpcTask/RpcTaskSource.h"
#include<google/protobuf/util/json_util.h>
#include"Component/Scene/NetThreadComponent.h"
namespace Sentry
{
	void InnerNetComponent::Awake()
	{
        this->mNetComponent = nullptr;
        this->mMessageComponent = nullptr;
    }
	bool InnerNetComponent::LateAwake()
	{
        LOG_CHECK_RET_FALSE(this->mNetComponent = this->GetComponent<NetThreadComponent>());
        LOG_CHECK_RET_FALSE(this->mMessageComponent = this->GetComponent<InnerNetMessageComponent>());
		return true;
	}

    void InnerNetComponent::OnMessage(const std::string &address, std::shared_ptr<Tcp::RpcMessage> message)
    {
        int len = 0;
        const char * data = message->GetData(len);
        MESSAGE_TYPE type = (MESSAGE_TYPE)message->GetType();
        MESSAGE_PROTO proto = (MESSAGE_PROTO)message->GetPorot();
        switch(proto)
        {
            case MESSAGE_PROTO::MSG_RPC_JSON:
            {

            }
                break;
            case MESSAGE_PROTO::MSG_RPC_PROTOBUF:
            {
                if(type == MESSAGE_TYPE::MSG_RPC_REQUEST)
                {
                    if(!this->mMessageComponent->OnProtoRequest(address, data, len))
                    {
                        this->StartClose(address);
                        return;
                    }
                }
                else if(type == MESSAGE_TYPE::MSG_RPC_RESPONSE)
                {
                    if(!this->mMessageComponent->OnProtoResponse(address, data, len))
                    {
                        this->StartClose(address);
                        return;
                    }
                }
            }
                break;
        }
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
			auto rpcClient = iter->second;

			rpcClient->StartClose();
			this->mRpcClientMap.erase(iter);
		}
	}

	std::shared_ptr<InnerNetClient> InnerNetComponent::GetOrCreateSession(const std::string& address)
	{
		std::shared_ptr<InnerNetClient> localSession = this->GetSession(address);
		if (localSession != nullptr)
		{
			return localSession;
		}
		std::string ip;
		unsigned short port = 0;
		assert(Helper::String::ParseIpAddress(address, ip, port));
		std::shared_ptr<SocketProxy> socketProxy = this->mNetComponent->CreateSocket();
        if(socketProxy == nullptr)
        {
            return nullptr;
        }
        socketProxy->Init(ip, port);
		localSession = make_shared<InnerNetClient>(this, socketProxy);

		this->mRpcClientMap.emplace(socketProxy->GetAddress(), localSession);
		return localSession;
	}

	std::shared_ptr<InnerNetClient> InnerNetComponent::GetSession(const std::string& address)
	{
		auto iter = this->mRpcClientMap.find(address);
		if (iter == this->mRpcClientMap.end())
		{
			return nullptr;
		}
		return iter->second;
	}


	bool InnerNetComponent::Send(const std::string & address, std::shared_ptr<com::rpc::request> message)
	{
		auto clientSession = this->GetOrCreateSession(address);
		if (message == nullptr || clientSession == nullptr)
		{
			return false;
		}
		clientSession->SendToServer(message);
		return true;
	}

	bool InnerNetComponent::Send(const std::string & address, std::shared_ptr<com::rpc::response> message)
	{
		std::shared_ptr<InnerNetClient> clientSession = this->GetSession(address);
		if (clientSession == nullptr || message == nullptr)
		{
			LOG_ERROR("send message to [" << address << "] failure");
			return false;
		}
		clientSession->SendToServer(message);
		return true;
	}
}// namespace Sentry