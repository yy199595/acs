//
// Created by mac on 2021/11/28.
//

#include"GateMessageClient.h"
#include"Message/c2s.pb.h"
#include"Util/TimeHelper.h"
#include"App/App.h"
#ifdef __DEBUG__
#include"google/protobuf/util/json_util.h"
#endif
#include"Network/Proto/RpcProtoMessage.h"
#include"Component/Gate/RpcGateComponent.h"

using namespace Tcp::Rpc;
namespace Sentry
{
	GateMessageClient::GateMessageClient(std::shared_ptr<SocketProxy> socket,
                                         RpcGateComponent* component)
		: TcpContext(socket), mGateComponent(component)
	{
		this->mQps = 0;
		this->mCallCount = 0;
	}

	void GateMessageClient::StartReceive(int second)
	{
#ifdef ONLY_MAIN_THREAD
		this->ReceiveLength();
#else
        asio::io_service & t = this->mSocket->GetThread();
        t.post(std::bind(&GateMessageClient::ReceiveLength, this));
        if(second != 0)
        {
            t.post(std::bind(&GateMessageClient::StartTimer, this, second));
        }
#endif
	}

    void GateMessageClient::StartTimer(int second)
    {
        auto timeout = std::chrono::seconds(second);
        asio::io_service & io = this->mSocket->GetThread();
        this->mTimer = std::make_shared<asio::steady_timer>(io, timeout);
        this->mTimer->async_wait(std::bind(&GateMessageClient::OnTimerEnd, this, second));
    }

    void GateMessageClient::OnTimerEnd(int timeout)
    {
        long long nowTime = Helper::Time::GetNowSecTime();
        if (nowTime - this->GetLastOperTime() < timeout) //超时
        {
            this->StartTimer(timeout);
            return;
        }
        this->CloseSocket(XCode::NetTimeout);
    }

    void GateMessageClient::OnReceiveLength(const asio::error_code &code, int length)
    {
        if(code || length <= 0)
        {
#ifdef __NET_ERROR_LOG__
            CONSOLE_LOG_ERROR(code.message());
#endif
            this->CloseSocket(XCode::NetReceiveFailure);
            return;
        }
        this->ReceiveMessage(length);
    }

    void GateMessageClient::OnReceiveMessage(const asio::error_code &code, std::istream & readStream, size_t size)
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
            std::shared_ptr<c2s::rpc::request> request(new c2s::rpc::request());
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
            asio::io_service &mainTaskScheduler = App::Get()->GetThread();
            mainTaskScheduler.post(std::bind(&RpcGateComponent::OnRequest, this->mGateComponent, request));
#endif
        }
        else
        {
            CONSOLE_LOG_FATAL("unknow message type");
            this->CloseSocket(XCode::UnKnowPacket);
            return;
        }
        this->ReceiveLength();
    }

	void GateMessageClient::CloseSocket(XCode code)
	{
        this->mSocket->Close();
        const std::string & address = this->GetAddress();
#ifdef ONLY_MAIN_THREAD
		this->mGateComponent->OnCloseSocket(address, code);
#else
		asio::io_service &mainTaskScheduler = App::Get()->GetThread();
		mainTaskScheduler.post(std::bind(&RpcGateComponent::OnCloseSocket, this->mGateComponent, address, code));
#endif
	}

	void GateMessageClient::OnSendMessage(const asio::error_code& code, std::shared_ptr<ProtoMessage> message)
	{
		if(code)
		{
#ifdef __NET_ERROR_LOG__
			CONSOLE_LOG_ERROR(code.message());
#endif
            this->PopAllMessage();
			this->CloseSocket(XCode::SendMessageFail);
            return;
		}
        this->PopMessage();
        this->SendFromMessageQueue();
	}

	void GateMessageClient::StartClose()
	{
		XCode code = XCode::NetActiveShutdown;
#ifdef ONLY_MAIN_THREAD
		this->CloseSocket(code);
#else
        asio::io_service & t = this->mSocket->GetThread();
        t.post(std::bind(&GateMessageClient::CloseSocket, this, code));
#endif
	}

	void GateMessageClient::SendToClient(std::shared_ptr<c2s::rpc::call> message)
	{
		std::shared_ptr<Tcp::Rpc::RpcProtoMessage> requestMessage
				= std::make_shared<Tcp::Rpc::RpcProtoMessage>(MESSAGE_TYPE::MSG_RPC_CALL_CLIENT, message);
#ifdef ONLY_MAIN_THREAD
		this->Send(requestMessage);
#else
        asio::io_service & t = this->mSocket->GetThread();
        t.post(std::bind(&GateMessageClient::Send, this, requestMessage));
#endif
	}

	void GateMessageClient::SendToClient(std::shared_ptr<c2s::rpc::response> message)
	{
		std::shared_ptr<Tcp::Rpc::RpcProtoMessage> requestMessage
				= std::make_shared<Tcp::Rpc::RpcProtoMessage>(MESSAGE_TYPE::MSG_RPC_RESPONSE, message);
#ifdef ONLY_MAIN_THREAD
		this->Send(requestMessage);
#else
        asio::io_service & t = this->mSocket->GetThread();
        t.post(std::bind(&GateMessageClient::Send, this, requestMessage));
#endif
        //CONSOLE_LOG_ERROR("send to client [" << this->mSocket->GetAddress() << "] " << message->rpc_id());
    }

}