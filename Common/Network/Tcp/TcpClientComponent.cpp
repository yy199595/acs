#include "TcpClientComponent.h"

#include <Core/App.h>
#include <Scene/CallHandlerComponent.h>
#include <Util/StringHelper.h>
#include <Scene/ProtocolComponent.h>
#include <Service/ServiceMgrComponent.h>
#include <Network/Tcp/TcpLocalSession.h>
#ifdef __DEBUG__
#include <Pool/MessagePool.h>
#endif
namespace GameKeeper
{
    TcpClientComponent::TcpClientComponent() = default;
    bool TcpClientComponent::Awake()
    {
		GKAssertRetFalse_F(this->mProtocolComponent = this->GetComponent<ProtocolComponent>());
		GKAssertRetFalse_F(this->mServiceComponent = this->GetComponent<ServiceMgrComponent>());
        GKAssertRetFalse_F(this->mCallHandlerComponent = this->GetComponent<CallHandlerComponent>());

        return true;
    }

	void TcpClientComponent::OnCloseSession(TcpClientSession * socket)
	{
		if (socket == nullptr)
		{
			return;
		}
		const std::string & address = socket->GetAddress();
		auto iter = this->mSessionAdressMap.find(address);
		if (iter != this->mSessionAdressMap.end())
		{
			this->mSessionAdressMap.erase(iter);
			GKDebugError("remove tcp socket " << address);
			delete socket;
		}
	}

    void TcpClientComponent::OnSendMessageAfter(TcpClientSession *session, const std::string &message,
                                                const asio::error_code &err)
    {
#ifdef __DEBUG__
        const std::string & address = session->GetAddress();
        auto type = (DataMessageType)message.at(sizeof(unsigned int));
        if(type == DataMessageType::TYPE_REQUEST)
        {
            GKDebugWarning(address << " send request message successful size = " << message.size());
        }
        else if(type == DataMessageType::TYPE_RESPONSE)
        {
            GKDebugWarning(address << " send response message successful size = " << message.size());
        }
#endif
        SocketHandler<TcpClientSession>::OnSendMessageAfter(session, message, err);
    }


	void TcpClientComponent::OnSessionError(TcpClientSession * clientSession, const  asio::error_code & err)
	{
		if (err)
		{
			const std::string & address = clientSession->GetAddress();
			if (clientSession->GetSocketType() == LocalSocket)
            {
                auto localSession = static_cast<TcpLocalSession *>(clientSession);
                if (localSession != nullptr)
                {
                    GKDebugError(
                            "[" << localSession->GetName() << ":" << address << "]" << " error" << err.message());
                }
            }
			else
			{
				auto iter = this->mSessionAdressMap.find(address);
				if (iter != this->mSessionAdressMap.end())
				{
					this->mSessionAdressMap.erase(iter);
					GKDebugError("remove tcp socket " << address);
					delete clientSession;
				}
			}
		}
	}


	void TcpClientComponent::OnConnectRemoteAfter(TcpClientSession *session, const asio::error_code &err)
	{
		const std::string & address = session->GetAddress();
		if (err)
		{
			GKDebugError("connect to " << address << " failure : " << err.message());
		}
        else
        {
            auto localSession = static_cast<TcpLocalSession *>(session);
            GKDebugInfo(
                    "connect to [" << localSession->GetName() << ":" << localSession->GetAddress() << "] successful");
            this->mSessionAdressMap.emplace(address, session);
        }
	}

	void TcpClientComponent::OnListenNewSession(TcpClientSession *clientSession, const asio::error_code & err)
	{
        if(err)
        {
            delete clientSession;
            GKDebugError(err.message());
        }
        else
        {
            const std::string & address = clientSession->GetAddress();
            this->mSessionAdressMap.emplace(address, clientSession);
        }
	}

	bool TcpClientComponent::OnReceiveMessage(TcpClientSession *session, const string & message)
	{
        const char * body = message.c_str() + 1;
        const size_t bodySize = message.size() - 1;
        const std::string & address = session->GetAddress();
		auto messageType = (DataMessageType)message.at(0);
		if (messageType == DataMessageType::TYPE_REQUEST)
        {
            this->mRequestData.Clear();
            if (!this->mRequestData.ParseFromArray(body, bodySize))
            {
                return false;
            }
            unsigned short methodId = this->mRequestData.methodid();
            auto config = this->mProtocolComponent->GetProtocolConfig(methodId);
            if (config == nullptr)
            {
                return false;
            }
#ifdef __DEBUG__
            std::string json;
            const std::string & data = this->mRequestData.messagedata();
            const std::string method = config->ServiceName + "." + config->Method;
            if(!config->RequestMessage.empty())
            {
               Message * msg = MessagePool::NewByData(config->RequestMessage, data);
               util::MessageToJsonString(*msg, &json);
            }
            GKDebugLog("[request " << method << "] json = " << json);
#endif
            this->mRequestData.set_address(address);
            return this->mServiceComponent->OnRequestMessage(mRequestData);
        }
		else if (messageType == DataMessageType::TYPE_RESPONSE)
		{
            this->mResponseData.Clear();
            if(!this->mResponseData.ParseFromArray(body, bodySize))
            {
                return false;
            }
            unsigned short methodId = this->mResponseData.methodid();
            auto config = this->mProtocolComponent->GetProtocolConfig(methodId);
            if (config == nullptr)
            {
                return false;
            }
#ifdef __DEBUG__
            std::string json;
            const std::string & data = this->mRequestData.messagedata();
            const std::string method = config->ServiceName + "." + config->Method;
            if(!config->ResponseMessage.empty())
            {
                Message * msg = MessagePool::NewByData(config->ResponseMessage, data);
                util::MessageToJsonString(*msg, &json);
            }
            GKDebugLog("[response " << method << "] code:" << this->mResponseData.code() << "  json = " << json);
#endif
            return this->mCallHandlerComponent->OnResponseMessage(mResponseData);
		}
        return false;
	}

    TcpLocalSession *TcpClientComponent::GetLocalSession(const std::string &address)
    {
        auto iter = this->mSessionAdressMap.find(address);
        if(iter == this->mSessionAdressMap.end())
        {
            return nullptr;
        }
        TcpClientSession * session = iter->second;
        if(session->GetSocketType() == SocketType::RemoteSocket)
        {
            return nullptr;
        }
        return static_cast<TcpLocalSession*>(session);
    }

    TcpClientSession *TcpClientComponent::GetRemoteSession(const std::string &address)
    {
        auto iter = this->mSessionAdressMap.find(address);
        return iter == this->mSessionAdressMap.end() ? nullptr : iter->second;
    }

    TcpLocalSession *TcpClientComponent::NewSession(const std::string &name, const std::string &ip,
                                                     unsigned short port)
	{
		std::string address = ip + ":" + std::to_string(port);
		auto iter = this->mSessionAdressMap.find(address);
		if (iter != this->mSessionAdressMap.end())
		{
			return static_cast<TcpLocalSession*>(iter->second);
		}
		AsioContext & io = this->GetNetThread()->GetContext();
		SharedTcpSocket socket = make_shared<AsioTcpSocket>(io);
		return new TcpLocalSession(this, name, ip, port);
	}

    void TcpClientComponent::OnDestory()
    {
    }

    TcpClientSession *TcpClientComponent::GetSession(const std::string &address)
    {
        auto iter = this->mSessionAdressMap.find(address);
        return iter != this->mSessionAdressMap.end() ? iter->second : nullptr;
    }

    bool TcpClientComponent::CloseSession(const std::string &address)
    {
        auto iter = this->mSessionAdressMap.find(address);
        if (iter != this->mSessionAdressMap.end())
        {
            TcpClientSession *session = iter->second;
            if (session != nullptr && session->IsActive())
            {
                session->Close();
            }			
            return true;
        }
        return false;
    }

	bool TcpClientComponent::SendByAddress(const std::string & address, std::string * message)
	{
		TcpClientSession * tcpSession = this->GetSession(address);
		if (tcpSession == nullptr)
		{
			return false;
		}
		return tcpSession->SendNetMessage(message);
	}

	bool TcpClientComponent::SendByAddress(const std::string & address, com::DataPacket_Request & message)
	{
        std::string * data = this->Serialize(message);
        if(data== nullptr)
        {
            return false;
        }
        return this->SendByAddress(address, data);
	}

	bool TcpClientComponent::SendByAddress(const std::string & address, com::DataPacket_Response & message)
	{
        std::string * data = this->Serialize(message);
        if(data== nullptr)
        {
            return false;
        }
		return this->SendByAddress(address, data);
	}

    std::string *TcpClientComponent::Serialize(const com::DataPacket_Request &message)
    {
        DataMessageType type = TYPE_REQUEST;
        const size_t size = message.ByteSizeLong() + 5;
        memcpy(this->mMessageBuffer, &size, sizeof(unsigned int));
        memcpy(this->mMessageBuffer + 4, &type, sizeof(char));
        if (!message.SerializeToArray(this->mMessageBuffer + 5, 1024 * 1024 - 5))
        {
            return nullptr;
        }
        return this->mStringPool.New(this->mMessageBuffer, size);
    }

    std::string *TcpClientComponent::Serialize(const com::DataPacket_Response &message)
    {
        DataMessageType type = TYPE_RESPONSE;
        const size_t size = message.ByteSizeLong() + 5;
        memcpy(this->mMessageBuffer, &size, sizeof(unsigned int));
        memcpy(this->mMessageBuffer + 4, &type, sizeof(char));
        if (!message.SerializeToArray(this->mMessageBuffer + 5, 1024 * 1024 - 5))
        {
            return nullptr;
        }
        return this->mStringPool.New(this->mMessageBuffer, size);
    }

}// namespace GameKeeper