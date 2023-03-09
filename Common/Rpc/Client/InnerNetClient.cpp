#include"InnerNetClient.h"
#include"App/App.h"
#include"Client/Rpc.h"
#include"Guid/Guid.h"
#include"System/System.h"
namespace Sentry
{
	InnerNetClient::InnerNetClient(IRpc<Rpc::Packet> * component,
                                   std::shared_ptr<SocketProxy> socket)
		: TcpContext(socket, 1024 * 1024), mComponent(component)
    {
        this->mState = Tcp::DecodeState::Head;
        this->mSrvName = ServerConfig::Inst()->Name();
        ServerConfig::Inst()->GetLocation("rpc", this->mRpcLocation);
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

    void InnerNetClient::Send(std::shared_ptr<Rpc::Packet> message)
    {
#ifdef ONLY_MAIN_THREAD
        this->Write(message);
#else
        asio::io_service &t = this->mSocket->GetThread();
        t.post(std::bind(&InnerNetClient::Write, this, message));
#endif
    }

	long long InnerNetClient::Call(std::shared_ptr<Rpc::Packet> message)
	{
		long long taskId = Guid::Create();
		message->GetHead().Add("rpc", taskId);
#ifdef ONLY_MAIN_THREAD
		this->Send(message);
#else
		asio::io_service &t = this->mSocket->GetThread();
		t.post(std::bind(&InnerNetClient::Write, this, message));
#endif
		return taskId;
	}

    void InnerNetClient::OnSendMessage(const asio::error_code& code, std::shared_ptr<ProtoMessage> message)
    {
#ifdef __DEBUG__
        if (code && this->mConnectCount > 0)
        {
            const std::string& address = this->mSocket->GetAddress();
            CONSOLE_LOG_ERROR("send inner message error : [" << address << "]");
        }
#endif
        if (code && this->mSocket->IsRemote())
        {
            this->Connect();            
        }
        else
        {
            this->PopMessage();
            this->SendFromMessageQueue();
        }
    }

    void InnerNetClient::Update()
    {
        if (Time::NowSecTime() - this->GetLastOperTime() >= 30) // 30s没动静
        {

        }
    }

    void InnerNetClient::CloseSocket(int code)
    {
        const std::string& address = this->mSocket->GetAddress();
        while (std::shared_ptr<ProtoMessage> data = this->PopMessage())
        {
            std::shared_ptr<Rpc::Packet> packet = std::static_pointer_cast<Rpc::Packet>(data);
            {
#ifdef ONLY_MAIN_THREAD
                this->mTcpComponent->OnMessage(address, std::move(packet));
#else
                asio::io_service& io = App::Inst()->MainThread();
                io.post(std::bind(&IRpc<Rpc::Packet>::OnSendFailure, this->mComponent, address, packet));
#endif
            }
        }
        if (code == XCode::NetActiveShutdown) //主动关闭不需要通知回主线
        {
            this->mSocket->Close();
            return;
        }
#ifdef ONLY_MAIN_THREAD
        this->mComponent->OnCloseSocket(address, code);
#else
        asio::io_service& taskScheduler = App::Inst()->MainThread();
        taskScheduler.post(std::bind(&IRpc<Rpc::Packet>::OnCloseSocket, this->mComponent, address, code));
#endif
    }

    void InnerNetClient::OnReceiveMessage(const asio::error_code &code, std::istream & readStream, size_t size)
    {
        if (code)
        {
            //CONSOLE_LOG_ERROR(code.message());
#ifdef __DEBUG__
            const std::string& address = this->mSocket->GetAddress();
            CONSOLE_LOG_ERROR("receive inner message error : [" << address << "]");
#endif // 
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
                this->mMessage = std::make_shared<Rpc::Packet>();
                if(!this->mMessage->ParseLen(readStream, len))
                {
                    this->CloseSocket(XCode::UnKnowPacket);
                    return;
                }
                //size_t count = this->mSocket->GetSocket().available(code);
                //assert(count >= len);
                this->ReceiveMessage(len);
            }
                break;
            case Tcp::DecodeState::Body:
            {
                this->mState = Tcp::DecodeState::Head;
                const std::string& address = this->mSocket->GetAddress();
                if (!this->mMessage->Parse(address,readStream, size))
                {
                    this->CloseSocket(XCode::UnKnowPacket);
                    return;
                }
                this->ReceiveMessage(RPC_PACK_HEAD_LEN);
#ifdef ONLY_MAIN_THREAD
                this->mComponent->OnMessage(std::move(this->mMessage));
#else
                asio::io_service & io = App::Inst()->MainThread();
                io.post(std::bind(&IRpc<Rpc::Packet>::OnMessage, this->mComponent, std::move(this->mMessage)));
#endif
            }
                break;
        }
    }

	void InnerNetClient::StartReceive()
	{
#ifdef ONLY_MAIN_THREAD
		this->ReceiveMessage(RPC_PACK_HEAD_LEN);
#else
        Asio::Context & t = this->mSocket->GetThread();
        t.post(std::bind(&InnerNetClient::ReceiveMessage, this, RPC_PACK_HEAD_LEN));
#endif
	}

	void InnerNetClient::OnConnect(const asio::error_code& error, int count)
	{
        if (this->mTimer != nullptr)
        {
            asio::error_code code;
            this->mTimer->cancel(code);
            this->mTimer = nullptr;
        }
		if(error)
		{
            if (this->mConnectCount >= 5)
            {
                this->CloseSocket(XCode::NetConnectFailure);
                return;
            }
#ifdef __DEBUG__
            const std::string& address = this->mSocket->GetAddress();
            CONSOLE_LOG_ERROR("connect inner error : [" << address << "]");
#endif // 
			Asio::Context & context = this->mSocket->GetThread();
			this->mTimer = std::make_unique<asio::steady_timer>(context, std::chrono::seconds(5));
			this->mTimer->async_wait(std::bind(&InnerNetClient::Connect, this->shared_from_this()));
			return;
		}
        
		asio::io_service & io = App::Inst()->MainThread();
		const std::string & address = this->mSocket->GetAddress();
		io.post(std::bind(&IRpc<Rpc::Packet>::OnConnectSuccessful, this->mComponent, address));

        std::shared_ptr<Rpc::Packet> authMessage =
            Rpc::Packet::New(Tcp::Type::Auth, Tcp::Porto::Protobuf);
        {
			Rpc::Head & head = authMessage->GetHead();
            {
                head.Add("name", this->mSrvName);
                head.Add("user", this->mUserName);
                head.Add("passwd", this->mPassword);
                head.Add("rpc", this->mRpcLocation);                          
            }
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