//
// Created by mac on 2021/11/28.
//

#include"OuterNetTcpClient.h"
#include"XCode/XCode.h"
#include"Util/Time/TimeHelper.h"
#include"Entity/Actor/App.h"
#include"Gate/Component/OuterNetComponent.h"

namespace Tendo
{
	OuterNetTcpClient::OuterNetTcpClient(std::shared_ptr<Tcp::SocketProxy> socket,
                                   OuterNetComponent* component)
		: TcpContext(socket), mGateComponent(component)
	{
        this->mState = Tcp::DecodeState::Head;
	}

	void OuterNetTcpClient::StartReceive(int second)
	{
#ifdef ONLY_MAIN_THREAD
		this->ReceiveMessage(RPC_PACK_HEAD_LEN);
#else
        Asio::Context & t = this->mSocket->GetThread();
        t.post(std::bind(&OuterNetTcpClient::ReceiveMessage, this, RPC_PACK_HEAD_LEN));
        if(second > 0)
        {
            t.post(std::bind(&OuterNetTcpClient::StartTimer, this, second));
        }
#endif 
	}

	void OuterNetTcpClient::OnTimeOut()
	{
		Asio::Context & t = App::Inst()->MainThread();
		const std::string& address = this->mSocket->GetAddress();
		t.post(std::bind(&OuterNetComponent::OnTimeout, this->mGateComponent, address));
	}

    void OuterNetTcpClient::OnReceiveMessage(const asio::error_code &code, std::istream & readStream, size_t size)
	{
		if (code)
		{
#ifdef __DEBUG__
			//CONSOLE_LOG_ERROR(code.message());
			//const std::string& address = this->mSocket->GetAddress();
			//CONSOLE_LOG_ERROR("receive outer message error : [" << address << "]");
#endif
			this->CloseSocket(XCode::NetReceiveFailure);
			return;
		}
		switch (this->mState)
		{
			case Tcp::DecodeState::Head:
			{
				int len = 0;
				this->mState = Tcp::DecodeState::Body;
				this->mMessage = std::make_shared<Msg::Packet>();
				if (!this->mMessage->ParseLen(readStream, len))
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
				const std::string& address = this->mSocket->GetAddress();
				if (!this->mMessage->Parse(address, readStream, size))
				{
					this->CloseSocket(XCode::UnKnowPacket);
					return;
				}
				this->ReceiveMessage(RPC_PACK_HEAD_LEN);
#ifdef ONLY_MAIN_THREAD
				this->mGateComponent->OnMessage(std::move(this->mMessage));
#else
				Asio::Context& io = App::Inst()->MainThread();
				io.post(std::bind(&OuterNetComponent::OnMessage,
					this->mGateComponent, this->mMessage));
#endif
			}
				break;
		}
	}

    void OuterNetTcpClient::CloseSocket(int code)
    {
		this->StopTimer();
		this->mSocket->Close();
        const std::string& address = this->GetAddress();
#ifdef ONLY_MAIN_THREAD
        this->mGateComponent->OnCloseSocket(address, code);
#else
        Asio::Context& mainTaskScheduler = App::Inst()->MainThread();
        mainTaskScheduler.post(std::bind(&OuterNetComponent::OnCloseSocket, this->mGateComponent, address, code));
#endif
        }

	void OuterNetTcpClient::OnSendMessage(const asio::error_code& code, std::shared_ptr<ProtoMessage> message)
	{
		if(code)
		{
#ifdef __DEBUG__
			//CONSOLE_LOG_ERROR(code.message());
            const std::string& address = this->mSocket->GetAddress();
            CONSOLE_LOG_ERROR("send outer message error : [" << address << "]");
#endif
            this->PopAllMessage();
			this->CloseSocket(XCode::SendMessageFail);
            return;
		}
        this->PopMessage();
        this->SendFromMessageQueue();
	}

	void OuterNetTcpClient::StartClose()
	{
		int code = XCode::NetActiveShutdown;
#ifdef ONLY_MAIN_THREAD
		this->CloseSocket(code);
#else
        Asio::Context & t = this->mSocket->GetThread();
        t.post(std::bind(&OuterNetTcpClient::CloseSocket, this, code));
#endif
	}

	void OuterNetTcpClient::SendData(std::shared_ptr<Msg::Packet> message)
	{
#ifdef ONLY_MAIN_THREAD
		this->Write(message);
#else
        Asio::Context & t = this->mSocket->GetThread();
        t.post(std::bind(&OuterNetTcpClient::Write, this, message));
#endif
	}
}