//
// Created by mac on 2021/11/28.
//

#include"OuterNetClient.h"
#include"Message/c2s.pb.h"
#include"Time/TimeHelper.h"
#include"App/App.h"
#ifdef __DEBUG__
#include"google/protobuf/util/json_util.h"
#endif
#include"Client/Message.h"
#include"Component/OuterNetComponent.h"

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
        Asio::Context & t = this->mSocket->GetThread();
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
        Asio::Context & io = this->mSocket->GetThread();
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
                int len = 0;
                this->mState = Tcp::DecodeState::Body;
                this->mMessage = std::make_shared<Rpc::Data>();
                if(!this->mMessage->ParseLen(readStream, len))
                {
                    this->CloseSocket(XCode::UnKnowPacket);
                    return;
                }
                this->ReceiveMessage(len);
            }
                break;
            case Tcp::DecodeState::Body:
            {
                this->mState = Tcp::DecodeState::Head;
                if (!this->mMessage->Parse(readStream, size))
                {
                    this->CloseSocket(XCode::UnKnowPacket);
                    return;
                }
                this->ReceiveMessage(RPC_PACK_HEAD_LEN);
                const std::string &address = this->mSocket->GetAddress();
#ifdef ONLY_MAIN_THREAD
                this->mGateComponent->OnMessage(address, std::move(this->mMessage));
#else
                Asio::Context &io = App::Inst()->GetThread();
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
		Asio::Context &mainTaskScheduler = App::Inst()->GetThread();
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
        Asio::Context & t = this->mSocket->GetThread();
        t.post(std::bind(&OuterNetClient::CloseSocket, this, code));
#endif
	}

	void OuterNetClient::SendData(std::shared_ptr<Rpc::Data> message)
	{
#ifdef ONLY_MAIN_THREAD
		this->Send(message);
#else
        Asio::Context & t = this->mSocket->GetThread();
        t.post(std::bind(&OuterNetClient::Send, this, message));
#endif
	}
}