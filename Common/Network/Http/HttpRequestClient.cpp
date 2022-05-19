//
// Created by yjz on 2022/1/19.
//
#include"HttpRequestClient.h"
#include"Http.h"
#include<regex>
namespace Sentry
{
    HttpRequestClient::HttpRequestClient(std::shared_ptr<SocketProxy> socketProxy)
		: Tcp::TcpContext(socketProxy)
    {

    }

	void HttpRequestClient::OnSendMessage(const asio::error_code& code, std::shared_ptr<Tcp::ProtoMessage> message)
	{
		if(code)
		{
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR(code.message());
#endif
			this->mWriteTask->SetResult(false);
			return;
		}
		this->mWriteTask->SetResult(true);
	}

    std::shared_ptr<HttpAsyncResponse> HttpRequestClient::Get(const std::string &url)
	{
		std::shared_ptr<HttpGetRequest> httpRequest = HttpGetRequest::Create(url);
		if(httpRequest == nullptr)
		{
			return nullptr;
		}
		return this->Request(httpRequest);
	}

    std::shared_ptr<HttpAsyncResponse> HttpRequestClient::Request(std::shared_ptr<HttpAsyncRequest> httpRequest)
    {
        const std::string & host = httpRequest->GetHost();
        const std::string & port = httpRequest->GetPort();
#ifdef ONLY_MAIN_THREAD
        this->ConnectHost(host, port, taskSource);
        if(taskSource->Await() != XCode::Successful)
        {
            LOG_ERROR("connect http host" << host << ":" << port <<  " failure");
            return nullptr;
        }
		LOG_DEBUG("connect http host " << host << ":" << port << " successful");

		std::shared_ptr<TaskSource<bool>> sendTaskSource(new TaskSource<bool>);
        this->SendByStream(httpRequest,sendTaskSource);

        if(!sendTaskSource->Await())
        {
            LOG_ERROR("send http get request failure");
            return nullptr;
        }
        std::shared_ptr<HttpAsyncResponse> httpContent(new HttpAsyncResponse());
        std::shared_ptr<TaskSource<bool>> recvTaskSource(new TaskSource<bool>);
        this->ReceiveHttpContent(recvTaskSource, httpContent);
        return recvTaskSource->Await() ? httpContent : nullptr;
#else
        IAsioThread & netWorkThread = this->mSocket->GetThread();
		this->mConnectTask = std::make_shared<TaskSource<bool>>();
        netWorkThread.Invoke(&HttpRequestClient::ConnectHost, this, host, port);
        if(!this->mConnectTask->Await())
        {
            CONSOLE_LOG_ERROR("connect http host " << host << ":" << port << " failure");
            return nullptr;
        }
		this->mWriteTask = std::make_shared<TaskSource<bool>>();
		LOG_INFO("connect http host " << host << ":" << port << " successful");
        netWorkThread.Invoke(&HttpRequestClient::Send, this, httpRequest);
        if(!this->mWriteTask->Await())
        {
			CONSOLE_LOG_ERROR("send http get request failure");
            return nullptr;
        }
		this->mReadTask = std::make_shared<TaskSource<bool>>();
        std::shared_ptr<HttpAsyncResponse> httpContent(new HttpAsyncResponse());
        netWorkThread.Invoke(&HttpRequestClient::ReceiveHttpContent, this, httpContent);
        return this->mReadTask->Await() ? httpContent : nullptr;
#endif
    }

    void HttpRequestClient::ReceiveHttpContent(std::shared_ptr<IHttpContent> httpContent)
    {
        AsioTcpSocket & tcpSocket = this->mSocket->GetSocket();
        asio::async_read(tcpSocket, this->mReadBuffer,
                         asio::transfer_at_least(1),[this, httpContent]
            (const asio::error_code & code, size_t size)
        {
            if(code)
            {
				CONSOLE_LOG_ERROR(code.message());
                this->mReadTask->SetResult(false);
                return;
            }
            switch(httpContent->OnReceiveData(this->mReadBuffer))
            {
                case HttpStatus::OK:
					this->mReadTask->SetResult(true);
                    break;
                case HttpStatus::CONTINUE:
                    this->ReceiveHttpContent(httpContent);
                    break;
                default:
					this->mReadTask->SetResult(false);
                    break;
            }
        });
    }

    std::shared_ptr<HttpAsyncResponse> HttpRequestClient::Post(const std::string &url, const std::string & content)
    {
        std::shared_ptr<HttpPostRequest> httpRequest = HttpPostRequest::Create(url);
		if(httpRequest == nullptr)
		{
			return nullptr;
		}
        httpRequest->AddBody(content);
        return this->Request(httpRequest);
    }

	std::shared_ptr<HttpAsyncResponse> HttpRequestClient::Post(const string& url, Json::Writer& content)
	{
		std::shared_ptr<HttpPostRequest> httpRequest = HttpPostRequest::Create(url);
		if(httpRequest == nullptr)
		{
			return nullptr;
		}
		httpRequest->AddBody(content.ToJsonString());
		httpRequest->AddHead("Content-Type", "application/json");
		return this->Request(httpRequest);
	}

    void HttpRequestClient::ConnectHost(const std::string & host, const std::string & port)
    {
        AsioContext & context = this->mSocket->GetContext();
        std::shared_ptr<asio::system_timer> connectTimer(new asio::system_timer(context, std::chrono::seconds(5)));
        connectTimer->async_wait([this](const asio::error_code & code)
        {
            if(!code)
            {
                this->mSocket->Close();
                this->mConnectTask->SetResult(false);
            }
        });
        std::shared_ptr<asio::ip::tcp::resolver> resolver(new asio::ip::tcp::resolver(context));
        std::shared_ptr<asio::ip::tcp::resolver::query> query(new asio::ip::tcp::resolver::query(host, port));
        resolver->async_resolve(*query, [this, resolver, query, connectTimer, port, host]
            (const asio::error_code &err, asio::ip::tcp::resolver::iterator iterator)
        {
            if(err)
            {
				this->mConnectTask->SetResult(false);
                return;
            }
            AsioTcpSocket & tcpSocket = this->mSocket->GetSocket();
            asio::async_connect(tcpSocket, iterator, [this, connectTimer]
                (const asio::error_code & code, asio::ip::tcp::resolver::iterator iter)
            {
                if(code)
                {
					this->mConnectTask->SetResult(false);
					return;
                }
                connectTimer->cancel();
				this->mConnectTask->SetResult(true);
            });
        });
    }
}
