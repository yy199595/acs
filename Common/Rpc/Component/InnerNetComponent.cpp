
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

    void InnerNetComponent::OnMessage(const std::string &address, std::shared_ptr<Rpc::Data> message)
    {
        switch ((Tcp::Type) message->GetType())
        {
            case Tcp::Type::Request:
                if (!this->OnRequest(address, message))
                {
                    this->StartClose(address);
                    CONSOLE_LOG_ERROR("request message error close : " << address);
                }
                break;
            case Tcp::Type::Response:
                if (!this->OnResponse(address, message))
                {
                    this->StartClose(address);
                    CONSOLE_LOG_ERROR("response message error close : " << address);
                }
                break;
            default:
            {
                std::string func;
                message->GetHead().Get("func", func);
                LOG_ERROR("call " << func << " type error");
            }
        }
    }

	bool InnerNetComponent::OnRequest(const std::string& address, std::shared_ptr<Rpc::Data> message)
	{
        message->GetHead().Add("address", address);
        LOG_CHECK_RET_FALSE(message->GetHead().Has("func"));
        XCode code = this->mMessageComponent->OnRequest(message);
        if(code != XCode::Successful)
        {
            std::string func;
            message->GetHead().Get("func", func);
            CONSOLE_LOG_ERROR("call " << func << " code = " << (int)code);
            return false;
        }
        return true;
	}

	bool InnerNetComponent::OnResponse(const std::string& address, std::shared_ptr<Rpc::Data> message)
	{
        int code = 0;
		long long rpcId = 0;
        const Rpc::Head & head = message->GetHead();
        LOG_CHECK_RET_FALSE(head.Get("rpc", rpcId));
        LOG_CHECK_RET_FALSE(head.Get("code", code));
        if(code != (int)XCode::Successful)
        {
#ifdef __DEBUG__
            std::string error, func;
            if(head.Get("func", func) && head.Get("error", error))
            {
                CONSOLE_LOG_ERROR("func = " << func << "  error = " << error);
            }
#endif
        }
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


	bool InnerNetComponent::Send(const std::string & address, std::shared_ptr<Rpc::Data> message)
	{
        assert(message->GetType() != Tcp::Type::None);
        assert(message->GetProto() != Tcp::Porto::None);
        std::shared_ptr<InnerNetClient> clientSession = this->GetOrCreateSession(address);
		if (message == nullptr || clientSession == nullptr)
		{
			return false;
		}
		clientSession->SendData(message);
		return true;
	}
}// namespace Sentry