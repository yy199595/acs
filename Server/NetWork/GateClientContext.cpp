//
// Created by mac on 2021/11/28.
//

#include"GateClientContext.h"
#include"Protocol/c2s.pb.h"
#include"App/App.h"
#ifdef __DEBUG__
#include"google/protobuf/util/json_util.h"
#endif
#include"Network/Proto/RpcProtoMessage.h"
#include"Component/Gate/GateClientComponent.h"

using namespace Tcp::Rpc;
namespace Sentry
{
	GateClientContext::GateClientContext(std::shared_ptr<SocketProxy> socket,
		GateClientComponent* component)
		: TcpContext(socket), mGateComponent(component)
	{
		this->mQps = 0;
		this->mCallCount = 0;
		this->SetBufferCount(2048, 2048);
	}

	void GateClientContext::StartReceive()
	{
#ifdef ONLY_MAIN_THREAD
		this->ReceiveHead();
#else
		this->mNetworkThread.Invoke(&GateClientContext::ReceiveHead, this);
#endif
	}

	bool GateClientContext::OnRecvMessage(const asio::error_code& code, const char* message, size_t size)
	{
		if(code)
		{
#ifdef __NET_ERROR_LOG__
			CONSOLE_LOG_ERROR(code.message());
#endif
			this->CloseSocket(XCode::NetReceiveFailure);
			return false;
		}

		if((MESSAGE_TYPE)message[0] == MESSAGE_TYPE::MSG_RPC_CLIENT_REQUEST)
		{
			std::shared_ptr<c2s::Rpc_Request> request(new c2s::Rpc_Request());
			if (!request->ParseFromArray(message + 1, (int)size - 1))
			{
				return false;
			}
			this->mCallCount++;
			this->mQps += size;
			request->set_address(this->GetAddress());
#ifdef ONLY_MAIN_THREAD
			this->mGateComponent->OnRequest(request);
#else
			MainTaskScheduler &mainTaskScheduler = App::Get()->GetTaskScheduler();
			mainTaskScheduler.Invoke(&GateClientComponent::OnRequest, this->mGateComponent, request);
#endif

			return true;
		}
		LOG_FATAL("client unknow message");
		return false;
	}

	void GateClientContext::CloseSocket(XCode code)
	{
		if (code == XCode::NetActiveShutdown)
		{
			this->mSocket->Close();
			return;
		}
		const std::string & address = this->GetAddress();
#ifdef ONLY_MAIN_THREAD
		this->mGateComponent->OnCloseSocket(address, code);
#else
		MainTaskScheduler &mainTaskScheduler = App::Get()->GetTaskScheduler();
		mainTaskScheduler.Invoke(&GateClientComponent::OnCloseSocket, this->mGateComponent, address, code);
#endif

	}

	void GateClientContext::OnSendMessage(const asio::error_code& code, std::shared_ptr<ProtoMessage> message)
	{
		if(code)
		{
#ifdef __NET_ERROR_LOG__
			CONSOLE_LOG_ERROR(code.message());
#endif
			this->CloseSocket(XCode::SendMessageFail);
			return;
		}
	}

	void GateClientContext::StartClose()
	{
		XCode code = XCode::NetActiveShutdown;
#ifdef ONLY_MAIN_THREAD
		this->CloseSocket(code);
#else
		this->mNetworkThread.Invoke(&GateClientContext::CloseSocket, this, code);
#endif
	}

	bool GateClientContext::SendToClient(std::shared_ptr<c2s::Rpc::Call> message)
	{
		if (!this->IsOpen())
		{
			return false;
		}
		std::shared_ptr<Tcp::Rpc::RpcProtoMessage> requestMessage
				= std::make_shared<Tcp::Rpc::RpcProtoMessage>(MESSAGE_TYPE::MSG_RPC_CALL_CLIENT, message);
#ifdef ONLY_MAIN_THREAD
		this->Send(requestMessage);
#else
		this->mNetworkThread.Invoke(&GateClientContext::Send, this, requestMessage);
#endif
		return true;
	}

	bool GateClientContext::SendToClient(std::shared_ptr<c2s::Rpc::Response> message)
	{
		if(!this->IsOpen())
		{
			return false;
		}
		std::shared_ptr<Tcp::Rpc::RpcProtoMessage> requestMessage
				= std::make_shared<Tcp::Rpc::RpcProtoMessage>(MESSAGE_TYPE::MSG_RPC_RESPONSE, message);
#ifdef ONLY_MAIN_THREAD
		this->Send(requestMessage);
#else
		this->mNetworkThread.Invoke(&GateClientContext::Send, this, requestMessage);
#endif
		return true;
	}

}