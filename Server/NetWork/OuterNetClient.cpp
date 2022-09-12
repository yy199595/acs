//
// Created by mac on 2021/11/28.
//

#include"OuterNetClient.h"
#include"Message/c2s.pb.h"
#include"Util/TimeHelper.h"
#include"App/App.h"
#ifdef __DEBUG__
#include"google/protobuf/util/json_util.h"
#endif
#include"Network/Proto/RpcProtoMessage.h"
#include"Component/Gate/OuterNetComponent.h"

using namespace Tcp::Rpc;
namespace Sentry
{
	OuterNetClient::OuterNetClient(std::shared_ptr<SocketProxy> socket,
                                   OuterNetComponent* component)
		: TcpContext(socket), mGateComponent(component)
	{
		this->mQps = 0;
		this->mCallCount = 0;
        this->mState = Tcp::DecodeState::Head;
	}

	void OuterNetClient::StartReceive(int second)
	{
#ifdef ONLY_MAIN_THREAD
		this->ReceiveMessage(RPC_PACK_HEAD_LEN);
#else
        asio::io_service & t = this->mSocket->GetThread();
        t.post(std::bind(&OuterNetClient::ReceiveMessage, this, RPC_PACK_HEAD_LEN));
        if(second != 0)
        {
            t.post(std::bind(&OuterNetClient::StartTimer, this, second));
        }
#endif
	}

    void OuterNetClient::StartTimer(int second)
    {
        auto timeout = std::chrono::seconds(second);
        asio::io_service & io = this->mSocket->GetThread();
        this->mTimer = std::make_shared<asio::steady_timer>(io, timeout);
        this->mTimer->async_wait(std::bind(&OuterNetClient::OnTimerEnd, this, second));
    }

    void OuterNetClient::OnTimerEnd(int timeout)
    {
        long long nowTime = Helper::Time::GetNowSecTime();
        if (nowTime - this->GetLastOperTime() < timeout) //超时
        {
            this->StartTimer(timeout);
            return;
        }
        this->CloseSocket(XCode::NetTimeout);
    }

    void OuterNetClient::OnReceiveMessage(const asio::error_code &code, std::istream & readStream, size_t size)
    {
        if (code)
        {
#ifdef __NET_ERROR_LOG__
            CONSOLE_LOG_ERROR(code.message());
#endif
            this->CloseSocket(XCode::NetReceiveFailure);
            return;
        }
        this->mQps += size;
        switch (this->mState)
        {
            case Tcp::DecodeState::Head:
            {
                this->mState = Tcp::DecodeState::Body;
                this->mMessage = std::make_shared<Tcp::RpcMessage>();
                int len = this->mMessage->DecodeHead(readStream);
                this->ReceiveMessage(len);
            }
                break;
            case Tcp::DecodeState::Body:
            {
                this->mState = Tcp::DecodeState::Head;
                if (!this->mMessage->DecodeBody(readStream))
                {
                    this->CloseSocket(XCode::UnKnowPacket);
                    return;
                }
                this->ReceiveMessage(RPC_PACK_HEAD_LEN);
                const std::string &address = this->mSocket->GetAddress();
#ifdef ONLY_MAIN_THREAD
                this->mGateComponent->OnMessage(address, std::move(this->mMessage));
#else
                asio::io_service &io = App::Get()->GetThread();
                io.post(std::bind(&OuterNetComponent::OnMessage,
                                  this->mGateComponent, address, std::move(this->mMessage)));
#endif
            }
                break;
        }
    }

	void OuterNetClient::CloseSocket(XCode code)
	{
        this->mSocket->Close();
        const std::string & address = this->GetAddress();
#ifdef ONLY_MAIN_THREAD
		this->mGateComponent->OnCloseSocket(address, code);
#else
		asio::io_service &mainTaskScheduler = App::Get()->GetThread();
		mainTaskScheduler.post(std::bind(&OuterNetComponent::OnCloseSocket, this->mGateComponent, address, code));
#endif
	}

	void OuterNetClient::OnSendMessage(const asio::error_code& code, std::shared_ptr<ProtoMessage> message)
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

	void OuterNetClient::StartClose()
	{
		XCode code = XCode::NetActiveShutdown;
#ifdef ONLY_MAIN_THREAD
		this->CloseSocket(code);
#else
        asio::io_service & t = this->mSocket->GetThread();
        t.post(std::bind(&OuterNetClient::CloseSocket, this, code));
#endif
	}

	void OuterNetClient::SendToClient(std::shared_ptr<c2s::rpc::call> message)
	{
		std::shared_ptr<Tcp::Rpc::RpcProtoMessage> request
				= std::make_shared<Tcp::Rpc::RpcProtoMessage>();

        request->mMessage = message;
        request->mType = Tcp::Type::Request;
#ifdef ONLY_MAIN_THREAD
		this->Send(request);
#else
        asio::io_service & t = this->mSocket->GetThread();
        t.post(std::bind(&OuterNetClient::Send, this, request));
#endif
	}

	void OuterNetClient::SendToClient(std::shared_ptr<c2s::rpc::response> message)
	{
		std::shared_ptr<Tcp::Rpc::RpcProtoMessage> response
				= std::make_shared<Tcp::Rpc::RpcProtoMessage>();

        response->mMessage = message;
        response->mType = Tcp::Type::Response;
#ifdef ONLY_MAIN_THREAD
		this->Send(response);
#else
        asio::io_service & t = this->mSocket->GetThread();
        t.post(std::bind(&OuterNetClient::Send, this, response));
#endif
        //CONSOLE_LOG_ERROR("send to client [" << this->mSocket->GetAddress() << "] " << message->rpc_id());
    }

}