//
// Created by zmhy0073 on 2022/1/19.
//

#include"HttpHandlerClient.h"
#include"Component/HttpListenComponent.h"

namespace Sentry
{
	HttpHandlerClient::HttpHandlerClient(HttpListenComponent * httpComponent, std::shared_ptr<SocketProxy> socketProxy)
		: Tcp::TcpContext(socketProxy)
	{
		this->mHttpComponent = httpComponent;
        this->mDecodeState = Http::DecodeState::None;
    }

	void HttpHandlerClient::StartReceive()
	{
        assert(this->mRecvBuffer.size() == 0);
        assert(this->mSendBuffer.size() == 0);
        this->mDecodeState = Http::DecodeState::None;
        this->mHttpResponse = std::make_shared<Http::Response>();
#ifdef ONLY_MAIN_THREAD
		this->ReceiveLine();
#else
		Asio::Context & netWorkThread = this->mSocket->GetThread();
		netWorkThread.post(std::bind(&HttpHandlerClient::ReceiveLine, this));
#endif
	}

    void HttpHandlerClient::StartWriter()
    {
#ifdef ONLY_MAIN_THREAD
        this->Send(this->mHttpResponse);
#else
        Asio::Context & netWorkThread = this->mSocket->GetThread();
        netWorkThread.post(std::bind(&HttpHandlerClient::Send, this, this->mHttpResponse));
#endif
    }

    void HttpHandlerClient::OnReceiveLine(const Asio::Code &code, std::istream &is, size_t size)
    {
        if(code && code != asio::error::eof)
        {
            this->ClosetClient();
        }
        else
        {
            is >> this->mMethod;
            this->mHttpRequest = Http::New(this->mMethod);
            if (this->mHttpRequest == nullptr)
            {
                this->mHttpResponse->Str(HttpStatus::METHOD_NOT_ALLOWED, "unknow method");
                this->Send(std::move(this->mHttpResponse));
                return;
            }
            this->ReceiveSomeMessage();
        }
    }

    void HttpHandlerClient::OnReceiveMessage(const asio::error_code &code, std::istream & is, size_t size)
    {
        if(code && code != asio::error::eof)
        {
            this->ClosetClient();
        }
        else if(code == asio::error::eof)
        {
END_RECEIVE:
            std::shared_ptr<HttpHandlerClient> httpHandlerClient =
                std::dynamic_pointer_cast<HttpHandlerClient>(this->shared_from_this());
#ifdef ONLY_MAIN_THREAD
            this->mHttpComponent->OnRequest(httpHandlerClient);
#else
            Asio::Context &mainThread = App::Inst()->GetThread();
            mainThread.post(std::bind(&HttpListenComponent::OnRequest, this->mHttpComponent, httpHandlerClient));
#endif
        }
        else
        {
            if(!this->mHttpRequest->OnRead(is))
            {
                this->ReceiveSomeMessage();
                return;
            }
            else
            {
                this->ClearRecvStream();
                goto END_RECEIVE;
            }
        }
    }

	void HttpHandlerClient::OnSendMessage(const asio::error_code& code, std::shared_ptr<Tcp::ProtoMessage> message)
	{
		if(code)
		{
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR(code.message());
#endif
		}
        this->PopMessage();
        this->ClosetClient();
    }
	void HttpHandlerClient::ClosetClient()
	{
        this->mSocket->Close();
        const std::string & address = this->mSocket->GetAddress();
#ifdef ONLY_MAIN_THREAD
		this->mHttpComponent->ClosetHttpClient(address);
#else
		Asio::Context & mainThread = App::Inst()->GetThread();
		mainThread.post(std::bind(&HttpListenComponent::ClosetHttpClient, this->mHttpComponent, address));
#endif
	}
}