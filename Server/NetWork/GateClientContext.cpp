//
// Created by mac on 2021/11/28.
//

#include"GateClientContext.h"
#include"Message/c2s.pb.h"
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
	}

	void GateClientContext::StartReceive()
	{
#ifdef ONLY_MAIN_THREAD
		this->ReceiveLength();
#else
		this->mNetworkThread.Invoke(&GateClientContext::ReceiveLength, this);
#endif
	}

    void GateClientContext::OnReceiveLength(const asio::error_code &code, int length)
    {
        if(code)
        {
#ifdef __NET_ERROR_LOG__
            CONSOLE_LOG_ERROR(code.message());
#endif
            this->CloseSocket(XCode::NetReceiveFailure);
            return;
        }
        this->ReceiveMessage(length);
    }

    void GateClientContext::OnReceiveMessage(const asio::error_code &code, std::istream & readStream)
    {
        if (code)
        {
#ifdef __NET_ERROR_LOG__
            CONSOLE_LOG_ERROR(code.message());
#endif
            this->CloseSocket(XCode::NetReceiveFailure);
            return;
        }
        if ((MESSAGE_TYPE) readStream.get() == MESSAGE_TYPE::MSG_RPC_CLIENT_REQUEST)
        {
            this->mQps += readStream.rdbuf()->in_avail();
            std::shared_ptr<c2s::Rpc_Request> request(new c2s::Rpc_Request());
            if (!request->ParseFromIstream(&readStream))
            {
                CONSOLE_LOG_ERROR("parse request message error");
                return;
            }
            this->mCallCount++;
            request->set_address(this->GetAddress());
#ifdef ONLY_MAIN_THREAD
            this->mGateComponent->OnRequest(request);
#else
            MainTaskScheduler &mainTaskScheduler = App::Get()->GetTaskScheduler();
            mainTaskScheduler.Invoke(&GateClientComponent::OnRequest, this->mGateComponent, request);
#endif
        }
        else
        {
            this->CloseSocket(XCode::UnKnowPacket);
            CONSOLE_LOG_FATAL("unknow message type");
            return;
        }
        this->ReceiveLength();
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
        this->SendFromMessageQueue();
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

	void GateClientContext::SendToClient(std::shared_ptr<c2s::Rpc::Call> message)
	{
		std::shared_ptr<Tcp::Rpc::RpcProtoMessage> requestMessage
				= std::make_shared<Tcp::Rpc::RpcProtoMessage>(MESSAGE_TYPE::MSG_RPC_CALL_CLIENT, message);
#ifdef ONLY_MAIN_THREAD
		this->Send(requestMessage);
#else
		this->mNetworkThread.Invoke(&GateClientContext::Send, this, requestMessage);
#endif
	}

	void GateClientContext::SendToClient(std::shared_ptr<c2s::Rpc::Response> message)
	{
		std::shared_ptr<Tcp::Rpc::RpcProtoMessage> requestMessage
				= std::make_shared<Tcp::Rpc::RpcProtoMessage>(MESSAGE_TYPE::MSG_RPC_RESPONSE, message);
#ifdef ONLY_MAIN_THREAD
		this->Send(requestMessage);
#else
		this->mNetworkThread.Invoke(&GateClientContext::Send, this, requestMessage);
#endif
        //CONSOLE_LOG_ERROR("send to client [" << this->mSocket->GetAddress() << "] " << message->rpc_id());
    }

}