//
// Created by zmhy0073 on 2022/1/19.
//

#include"HttpHandlerClient.h"
#include"Component/Http/HttpComponent.h"
namespace Sentry
{
	HttpHandlerClient::HttpHandlerClient(HttpComponent * httpComponent, std::shared_ptr<SocketProxy> socketProxy)
		: Tcp::TcpContext(socketProxy)
	{
		this->mHttpComponent = httpComponent;
		this->mHttpResponse = std::make_shared<HttpHandlerResponse>();
		this->mHttpRequest = std::make_shared<HttpHandlerRequest>(socketProxy->GetAddress());
	}

	void HttpHandlerClient::StartReceive()
	{
#ifdef ONLY_MAIN_THREAD
		this->ReadData();
#else
		IAsioThread & netWorkThread = this->mSocket->GetThread();
		netWorkThread.Invoke(&HttpHandlerClient::ReceiveSomeMessage, this);
#endif
	}

	void HttpHandlerClient::StartWriter()
	{
#ifdef ONLY_MAIN_THREAD
		this->Send(this->mHttpResponse);
#else
		IAsioThread& netWorkThread = this->mSocket->GetThread();
		netWorkThread.Invoke(&HttpHandlerClient::Send, this, this->mHttpResponse);
#endif
	}

	void HttpHandlerClient::OnComplete()
	{
		std::shared_ptr<HttpHandlerClient> httpHandlerClient =
			std::dynamic_pointer_cast<HttpHandlerClient>(this->shared_from_this());
#ifdef ONLY_MAIN_THREAD
		this->mHttpComponent->HandlerHttpData(httpHandlerClient);
#else
		IAsioThread& mainThread = App::Get()->GetTaskScheduler();
		mainThread.Invoke(&HttpComponent::OnRequest, this->mHttpComponent, httpHandlerClient);
#endif
	}

    void HttpHandlerClient::OnReceiveMessage(const asio::error_code &code, asio::streambuf &buffer)
    {
        assert(this->mHttpRequest);
        if(code == asio::error::eof)
        {
            this->OnComplete();
            return;
        }
        switch(this->mHttpRequest->OnReceiveSome(buffer))
        {
            case 0:
                this->OnComplete();
                break;
            case 1:
            case -1:
                this->ReceiveSomeMessage();
                break;
            default:
                this->ClosetClient();
                break;
        }
    }

	void HttpHandlerClient::OnSendMessage(const asio::error_code& code, std::shared_ptr<Tcp::ProtoMessage> message)
	{
		this->ClosetClient();
		if(code)
		{
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR(code.message());
#endif
			return;
		}
	}
	void HttpHandlerClient::ClosetClient()
	{
		this->mSocket->Close();
		std::move(this->mSocket);
		std::shared_ptr<HttpHandlerClient> httpHandlerClient =
			std::dynamic_pointer_cast<HttpHandlerClient>(this->shared_from_this());
#ifdef ONLY_MAIN_THREAD
		this->mHttpComponent->ClosetHttpClient(httpHandlerClient);
#else
		IAsioThread& mainThread = App::Get()->GetTaskScheduler();
		mainThread.Invoke(&HttpComponent::ClosetHttpClient, this->mHttpComponent, httpHandlerClient);
#endif
	}
}