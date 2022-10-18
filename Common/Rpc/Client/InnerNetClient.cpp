#include"InnerNetClient.h"
#include"App/App.h"
#include"Client/Rpc.h"
#include"App/System/System.h"
namespace Sentry
{
	InnerNetClient::InnerNetClient(IRpc<Rpc::Data> * component,
                                   std::shared_ptr<SocketProxy> socket)
		: TcpContext(socket, 1024 * 1024), mComponent(component)
	{
        this->mSrvName = System::GetName();
        this->mState = Tcp::DecodeState::Head;
        ServerConfig::Inst()->GetLocation("rpc", this->mLocation);
        ServerConfig::Inst()->GetMember("user", "name", this->mUserName);
        ServerConfig::Inst()->GetMember("user", "passwd", this->mPassword);
	}

	void InnerNetClient::StartClose()
	{
#ifdef ONLY_MAIN_THREAD
		this->CloseSocket(XCode::NetActiveShutdown);
#else
        Asio::Context & t = this->mSocket->GetThread();
		t.post(std::bind(&InnerNetClient::CloseSocket, this, XCode::NetActiveShutdown));
#endif
	}

    void InnerNetClient::SendData(std::shared_ptr<Rpc::Data> message)
    {
#ifdef ONLY_MAIN_THREAD
        this->Send(message);
#else
        asio::io_service &t = this->mSocket->GetThread();
        t.post(std::bind(&InnerNetClient::Send, this, message));
#endif
    }

	void InnerNetClient::OnSendMessage(const asio::error_code& code, std::shared_ptr<ProtoMessage> message)
	{
		if(code && this->mSocket->IsRemote())
		{
			this->Connect();
		}
        else
        {
            this->PopMessage();
            this->SendFromMessageQueue();
        }
	}

	void InnerNetClient::CloseSocket(XCode code)
	{
		if (code == XCode::NetActiveShutdown) //主动关闭不需要通知回主线
		{
			this->mSocket->Close();
			return;
		}
		const std::string & address = this->GetAddress();
#ifdef ONLY_MAIN_THREAD
		this->mTcpComponent->OnCloseSocket(address, code);
#else
		asio::io_service & taskScheduler = App::Inst()->GetThread();
		taskScheduler.post(std::bind(&IRpc<Rpc::Data>::OnCloseSocket, this->mComponent, address, code));
#endif
	}

    void InnerNetClient::OnReceiveMessage(const asio::error_code &code, std::istream & readStream, size_t size)
    {
        if (code)
        {
            CONSOLE_LOG_ERROR(code.message());
            this->CloseSocket(XCode::NetWorkError);
            return;
        }
        switch(this->mState)
        {
            case Tcp::DecodeState::Head:
            {
                int len = 0;
                Asio::Code code;
                this->mState = Tcp::DecodeState::Body;
                this->mMessage = std::make_shared<Rpc::Data>();
                if(!this->mMessage->ParseLen(readStream, len))
                {
                    this->CloseSocket(XCode::UnKnowPacket);
                    return;
                }
                size_t count = this->mSocket->GetSocket().available(code);
                assert(count >= len);
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
                const std::string & address = this->mSocket->GetAddress();
#ifdef ONLY_MAIN_THREAD
                this->mTcpComponent->OnMessage(address, std::move(this->mMessage));
#else
                asio::io_service & io = App::Inst()->GetThread();
                io.post(std::bind(&IRpc<Rpc::Data>::OnMessage, this->mComponent, address, std::move(this->mMessage)));
#endif
            }
                break;
        }
    }

	void InnerNetClient::StartReceive()
	{
#ifdef ONLY_MAIN_THREAD
		this->ReceiveMessage(PackeHeadtLength);
#else
        Asio::Context & t = this->mSocket->GetThread();
        t.post(std::bind(&InnerNetClient::ReceiveMessage, this, RPC_PACK_HEAD_LEN));
#endif
	}

	void InnerNetClient::OnConnect(const asio::error_code& error, int count)
	{
		if(error)
		{
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR(error.message());
#endif
			Asio::Context & context = this->mSocket->GetThread();
			this->mTimer = std::make_shared<asio::steady_timer>(context, std::chrono::seconds(5));
			this->mTimer->async_wait(std::bind(std::bind(&InnerNetClient::Connect, this->shared_from_this())));
			return;
		}

        std::shared_ptr<Rpc::Data> authMessage =
            Rpc::Data::New(Tcp::Type::Auth, Tcp::Porto::Protobuf);
        {
            authMessage->GetHead().Add("name", this->mSrvName);
            authMessage->GetHead().Add("user", this->mUserName);
            authMessage->GetHead().Add("passwd", this->mPassword);
            authMessage->GetHead().Add("location", this->mLocation);
        }
        if(this->SendSync(authMessage) > 0)
        {
            this->SendFromMessageQueue();
            this->ReceiveMessage(RPC_PACK_HEAD_LEN);
        }
        else
        {
            this->CloseSocket(XCode::NetWorkError);
        }
    }
}