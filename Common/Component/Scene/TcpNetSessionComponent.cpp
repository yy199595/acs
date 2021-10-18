#include "TcpNetSessionComponent.h"

#include <Core/App.h>
#include "ActionComponent.h"
#include "LuaScriptComponent.h"
#include <Util/StringHelper.h>
#include <Scene/ProtocolComponent.h>
#include <Service/ServiceMgrComponent.h>
#include <NetWork/TcpClientSession.h>
namespace Sentry
{
    TcpNetSessionComponent::TcpNetSessionComponent()
    {

    }


    bool TcpNetSessionComponent::Awake()
    {
		SayNoAssertRetFalse_F(this->mActionComponent = this->GetComponent<ActionComponent>());
		SayNoAssertRetFalse_F(this->mProtocolComponent = this->GetComponent<ProtocolComponent>());
		SayNoAssertRetFalse_F(this->mServiceComponent = this->GetComponent<ServiceMgrComponent>());
		

		std::vector<Component *> components;
		this->gameObject->GetComponents(components);
		for (Component * component : components)
		{
			if (auto reqHandler = dynamic_cast<IRequestMessageHandler*>(component))
			{
				this->mRequestMsgHandlers.emplace(component->GetTypeName(), reqHandler);
			}
			if (auto resHandler = dynamic_cast<IResponseMessageHandler*>(component))
			{
				this->mResponseMsgHandlers.emplace(component->GetTypeName(), resHandler);
			}
		}
		SayNoAssertRetFalse_F(!this->mRequestMsgHandlers.empty() && !this->mResponseMsgHandlers.empty());

        return true;
    }

	void TcpNetSessionComponent::OnClose(SessionBase * socket)
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
			SayNoDebugError("remove tcp socket " << address);
			delete socket;			
		}
	}

	SessionBase * TcpNetSessionComponent::CreateSocket(AsioContext & io)
	{
		return new TcpClientSession(this);
	}

	void TcpNetSessionComponent::OnSessionErr(SessionBase * session, const  asio::error_code & err)
	{
		if (err)
		{
			const std::string & address = session->GetAddress();
			SayNoDebugError("[" << address << "]" << " error", err.message());
			TcpClientSession * clientSession = static_cast<TcpClientSession*>(session);
			if (clientSession->IsConnected())
			{
				std::string ip;
				unsigned short port;
				StringHelper::ParseIpAddress(address, ip, port);
				const std::string & name = clientSession->GetSessionName();
				clientSession->StartConnect(name, ip, port);
			}
			else
			{
				auto iter = this->mSessionAdressMap.find(address);
				if (iter != this->mSessionAdressMap.end())
				{
					this->mSessionAdressMap.erase(iter);
					SayNoDebugError("remove tcp socket " << address);
					delete clientSession;
				}
			}
		}
	}

	void TcpNetSessionComponent::OnConnectRemote(SessionBase * session, const asio::error_code & err)
	{
		const std::string & address = session->GetAddress();
		if (err)
		{
			SayNoDebugError(err.message());
		}
	}

	void TcpNetSessionComponent::OnListenConnect(NetWorkThread * netTask, SessionBase * session)
	{
		TcpClientSession * clientSession = dynamic_cast<TcpClientSession*>(session);
		if (clientSession != nullptr)
		{
			const std::string & address = clientSession->GetAddress();
			auto iter = this->mSessionAdressMap.find(address);
			if (iter != this->mSessionAdressMap.end())
			{
				return;
			}
			this->mSessionAdressMap.emplace(address, clientSession);
		}
	}
	void TcpNetSessionComponent::OnReceiveNewMessage(SessionBase * session, SharedMessage message)
	{
		unsigned short methodId = 0;
		const char * msg = message->c_str();
		const size_t size = message->size();
		memcpy(&methodId, msg + 1, sizeof(methodId));
		const std::string & address = session->GetAddress();
		const ProtocolConfig * protocolConfig = this->mProtocolComponent->GetProtocolConfig(methodId);

		if (protocolConfig == nullptr)
		{
			this->CloseSession(address);
			return;
		}

		auto messageType = (DataMessageType)message->at(0);
		if (messageType == DataMessageType::TYPE_REQUEST)
		{
			const std::string &handler = protocolConfig->RequestHandler;
			auto iter = this->mRequestMsgHandlers.find(handler);
			if (iter != this->mRequestMsgHandlers.end())
			{
				if (!iter->second->OnRequestMessage(address, message))
				{
					this->CloseSession(address);
				}
				return;
			}
			this->mServiceComponent->OnRequestMessage(address, message);
		}
		else if (messageType == DataMessageType::TYPE_RESPONSE)
		{
			const std::string & handler = protocolConfig->RequestHandler;
			auto iter = this->mResponseMsgHandlers.find(handler);
			if (iter != this->mResponseMsgHandlers.end())
			{
				if (!iter->second->OnResponseMessage(address, message))
				{
					this->CloseSession(address);
				}
				return;
			}
			this->mActionComponent->OnResponseMessage(address, message);
		}
		else
		{
			this->CloseSession(address);
		}
		
	}

	TcpClientSession *TcpNetSessionComponent::ConnectRemote(const std::string &name, const std::string & ip, unsigned short port)
	{
		std::string address = ip + ":" + std::to_string(port);
		auto iter = this->mSessionAdressMap.find(address);
		if (iter != this->mSessionAdressMap.end())
		{
			return iter->second;
		}
		AsioContext & io = this->GetNetThread()->GetContext();
		SharedTcpSocket socket = make_shared<AsioTcpSocket>(io);
		TcpClientSession * clientSession = new TcpClientSession(this);

		clientSession->SetSocket(socket);
		clientSession->StartConnect(name, ip, port);
		return clientSession;


	}

    void TcpNetSessionComponent::OnDestory()
    {
    }

    TcpClientSession *TcpNetSessionComponent::GetSession(const std::string &address)
    {
        auto iter = this->mSessionAdressMap.find(address);
        return iter != this->mSessionAdressMap.end() ? iter->second : nullptr;
    }

    bool TcpNetSessionComponent::CloseSession(const std::string &address)
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

	bool TcpNetSessionComponent::SendByAddress(const std::string & address, SharedMessage message)
	{
		TcpClientSession * tcpSession = this->GetSession(address);
		if (tcpSession == nullptr)
		{
			return false;
		}
		return tcpSession->SendNetMessage(message);
	}

	bool TcpNetSessionComponent::SendByAddress(const std::string & address, com::DataPacket_Request & message)
	{
		unsigned short methodId = message.methodid();
		message.clear_methodid();
		return this->SendByAddress(address, TYPE_REQUEST, methodId, message);
	}

	bool TcpNetSessionComponent::SendByAddress(const std::string & address, com::DataPacket_Response & message)
	{
		unsigned short methodId = message.methodid();
		message.clear_methodid();
		return this->SendByAddress(address, TYPE_RESPONSE, methodId, message);
	}

	bool TcpNetSessionComponent::SendByAddress(const std::string & address, DataMessageType type, unsigned short methodId, Message & message)
	{
		// size + type + methodId + body
		const size_t size = message.ByteSizeLong() + 7;
		memcpy(this->mMessageBuffer, &size, sizeof(char));
		memcpy(this->mMessageBuffer + 4, &type, sizeof(char));
		memcpy(this->mMessageBuffer + 5, &methodId, sizeof(unsigned short));
		if (!message.ParseFromArray(this->mMessageBuffer + 7, 1024 * 1024 - 7))
		{
			return false;
		}
		return this->SendByAddress(address, std::make_shared<std::string>(this->mMessageBuffer, size));
	}
}// namespace Sentry