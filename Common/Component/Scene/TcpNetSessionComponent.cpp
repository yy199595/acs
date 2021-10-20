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
        return true;
    }

	void TcpNetSessionComponent::OnCloseSession(TcpClientSession * socket)
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

	void TcpNetSessionComponent::OnSessionError(TcpClientSession * clientSession, const  asio::error_code & err)
	{
		if (err)
		{
			const std::string & address = clientSession->GetAddress();
			SayNoDebugError("[" << address << "]" << " error" << err.message());
			if (clientSession->IsConnected())
			{
				std::string ip;
				unsigned short port;
                const std::string & name = clientSession->GetName();
				StringHelper::ParseIpAddress(address, ip, port);
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


	void TcpNetSessionComponent::OnConnectRemoteAfter(TcpClientSession *session, const asio::error_code &err)
	{
		const std::string & address = session->GetAddress();
		if (err)
		{
			SayNoDebugError(err.message());
		}
	}

	bool TcpNetSessionComponent::OnListenNewSession(TcpClientSession *clientSession)
	{
		if (clientSession != nullptr)
		{
			const std::string & address = clientSession->GetAddress();
			auto iter = this->mSessionAdressMap.find(address);
			if (iter != this->mSessionAdressMap.end())
			{
				return false;
			}
			this->mSessionAdressMap.emplace(address, clientSession);
		}
        return true;
	}

	bool TcpNetSessionComponent::OnReceiveMessage(TcpClientSession *session, const string & message)
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
            if (this->mProtocolComponent->GetProtocolConfig(methodId) == nullptr)
            {
                return false;
            }
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
            return this->mActionComponent->OnResponseMessage(mResponseData);
		}
        return false;
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

		auto clientSession = new TcpClientSession(this);
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

	bool TcpNetSessionComponent::SendByAddress(const std::string & address, std::string * message)
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
		return this->SendByAddress(address, TYPE_REQUEST, message);
	}

	bool TcpNetSessionComponent::SendByAddress(const std::string & address, com::DataPacket_Response & message)
	{
		return this->SendByAddress(address, TYPE_RESPONSE, message);
	}

	bool TcpNetSessionComponent::SendByAddress(const std::string & address, DataMessageType type, Message & message)
	{
		// size + type + + body
		const size_t size = message.ByteSizeLong() + 5;
		memcpy(this->mMessageBuffer, &size, sizeof(char));
		memcpy(this->mMessageBuffer + 4, &type, sizeof(char));
		if (!message.ParseFromArray(this->mMessageBuffer + 5, 1024 * 1024 - 5))
		{
			return false;
		}
		return true;//this->SendByAddress(address, this->mStringPool.New(this->mMessageBuffer, size));
	}
}// namespace Sentry