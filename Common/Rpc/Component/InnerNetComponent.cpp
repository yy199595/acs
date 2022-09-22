
#include"InnerNetComponent.h"
#include"App/App.h"
#include"String/StringHelper.h"
#include"Tcp/SocketProxy.h"
#include"InnerNetMessageComponent.h"
#include"Config/ServiceConfig.h"
#include"Component/NetThreadComponent.h"
#include"google/protobuf/util/json_util.h"
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

    void InnerNetComponent::OnMessage(const std::string &address, std::shared_ptr<Tcp::BinMessage> message)
    {
		switch ((Tcp::Type)message->GetType())
		{
		case Tcp::Type::Request:
			if (!this->OnRequest(address, *message))
			{
				this->StartClose(address);
			}
			break;
		case Tcp::Type::Response:
			if (!this->OnResponse(address, *message))
			{
				this->StartClose(address);
			}
			break;
		}      
    }

	bool InnerNetComponent::OnRequest(const std::string& address, const Tcp::BinMessage& message)
	{
		int len = 0;
		const char * data = message.GetData(len);
		std::shared_ptr<com::rpc::request> request
			= std::make_shared<com::rpc::request>();
		if (!request->ParseFromArray(data, len))
		{
			return false;
		}
		request->set_address(address);
		request->set_type(com::rpc_msg_type::rpc_msg_type_proto);
		return this->mMessageComponent->OnRequest(request) == XCode::Successful;
	}

	bool InnerNetComponent::OnResponse(const std::string& address, const Tcp::BinMessage& message)
	{
		int len = 0;
		const char* data = message.GetData(len);
		std::shared_ptr<com::rpc::response> response
			= std::make_shared<com::rpc::response>();
		if (!response->ParseFromArray(data, len))
		{
			return false;
		}
		long long rpcId = response->rpc_id();
		this->mMessageComponent->OnResponse(rpcId, response);
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
		localSession = std::make_shared<InnerNetClient>(this, socketProxy);

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