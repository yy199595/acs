//
// Created by zmhy0073 on 2022/1/19.
//

#include"HttpHandlerClient.h"
#include"Component/Http/HttpListenComponent.h"

namespace Sentry
{
	HttpHandlerClient::HttpHandlerClient(HttpListenComponent * httpComponent, std::shared_ptr<SocketProxy> socketProxy)
		: Tcp::TcpContext(socketProxy)
	{
		this->mHttpComponent = httpComponent;
	}

	void HttpHandlerClient::StartReceive()
	{
        assert(this->mRecvBuffer.size() == 0);
        assert(this->mSendBuffer.size() == 0);
        const std::string & address = this->mSocket->GetAddress();
        this->mHttpResponse = std::make_shared<HttpHandlerResponse>();
        this->mHttpRequest = std::make_shared<HttpHandlerRequest>(address);
#ifdef ONLY_MAIN_THREAD
		this->ReceiveLine();
#else
		asio::io_service & netWorkThread = this->mSocket->GetThread();
		netWorkThread.post(std::bind(&HttpHandlerClient::ReceiveLine, this));
#endif
	}

    void HttpHandlerClient::StartWriter(HttpStatus code)
    {
        this->mHttpResponse->SetCode(code);
        CONSOLE_LOG_ERROR("[" << this->mHttpRequest->GetPath() << "] " << HttpStatusToString(code));
#ifdef ONLY_MAIN_THREAD
        this->Send(this->mHttpResponse);
#else
        asio::io_service& netWorkThread = this->mSocket->GetThread();
        netWorkThread.post(std::bind(&HttpHandlerClient::Send, this, this->mHttpResponse));
#endif
    }

	void HttpHandlerClient::OnComplete()
	{
		std::shared_ptr<HttpHandlerClient> httpHandlerClient =
			std::dynamic_pointer_cast<HttpHandlerClient>(this->shared_from_this());
#ifdef ONLY_MAIN_THREAD
		this->mHttpComponent->OnRequest(httpHandlerClient);
#else
		asio::io_service& mainThread = App::Get()->GetThread();
		mainThread.post(std::bind(&HttpListenComponent::OnRequest, this->mHttpComponent, httpHandlerClient));
#endif
	}

    void HttpHandlerClient::OnReceiveLine(const asio::error_code &code, std::istream &is, size_t)
    {
        if(code == asio::error::eof)
        {
            this->OnComplete();
            return;
        }
        switch(this->mHttpRequest->OnReceiveLine(is))
        {
            case 0:
                this->OnComplete();
                break;
            case -1:
                this->ReceiveLine();
                break;
            case 1:
                this->ReceiveSomeMessage();
                break;
            default:
                this->ClosetClient();
                break;
        }
    }

    void HttpHandlerClient::OnReceiveMessage(const asio::error_code &code, std::istream & is, size_t)
    {
        if(code == asio::error::eof)
        {
            this->OnComplete();
            return;
        }
        switch(this->mHttpRequest->OnReceiveSome(is))
        {
            case 0:
                this->OnComplete();
                break;
            case 1:
                this->ReceiveSomeMessage();
                break;
            default:
                this->ClosetClient();
                break;
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
		asio::io_service& mainThread = App::Get()->GetThread();
		mainThread.post(std::bind(&HttpListenComponent::ClosetHttpClient, this->mHttpComponent, address));
#endif
	}
}