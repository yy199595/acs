//
// Created by yjz on 2022/1/19.
//
#include"HttpRequestClient.h"
#include"Http.h"
#include<regex>
namespace Sentry
{
    HttpRequestClient::HttpRequestClient(std::shared_ptr<SocketProxy> socketProxy, HttpComponent * httpComponent)
		: Tcp::TcpContext(socketProxy)
    {
		this->mHttpComponent = httpComponent;
    }

	void HttpRequestClient::OnSendMessage(const asio::error_code& code, std::shared_ptr<Tcp::ProtoMessage> message)
	{
		if(code)
		{
#ifdef __NET_ERROR_LOG__
			CONSOLE_LOG_ERROR(code.message());
#endif
			this->mHttpTask.SetResult(false);
			return;
		}
		this->mHttpTask.SetResult(true);
	}

    std::shared_ptr<HttpAsyncResponse> HttpRequestClient::Get(const std::string &url)
	{
		this->mRequest = HttpGetRequest::Create(url);
		if(this->mRequest == nullptr)
		{
			return nullptr;
		}
		return this->Request(this->mRequest);
	}

    std::shared_ptr<HttpAsyncResponse> HttpRequestClient::Request(std::shared_ptr<HttpAsyncRequest> httpRequest, std::fstream * fs)
    {
        const std::string & host = httpRequest->GetHost();
        const std::string & port = httpRequest->GetPort();
#ifdef ONLY_MAIN_THREAD
		this->ConnectHost(host, port);
        if(!this->mHttpTask.Await())
        {
            LOG_ERROR("connect http host" << host << ":" << port <<  " failure");
            return nullptr;
        }
		LOG_DEBUG("connect http host " << host << ":" << port << " successful");
		this->mHttpTask.Clear();
		this->Send(httpRequest);

        if(!this->mHttpTask.Await())
        {
            LOG_ERROR("send http get request failure");
            return nullptr;
        }
		this->mHttpTask.Clear();
		LOG_WARN("send http data to " << host << ":" << port << " successful");
		std::shared_ptr<HttpAsyncResponse> httpContent(new HttpAsyncResponse(nullptr));
        this->ReceiveHttpContent(httpContent);
		LOG_ERROR("recv http data from " << host << ":" << port << " successful");
		return this->mHttpTask.Await() ? httpContent : nullptr;
#else
        IAsioThread & netWorkThread = this->mSocket->GetThread();
        netWorkThread.Invoke(&HttpRequestClient::ConnectHost, this, host, port);
        if(!this->mHttpTask.Await())
        {
            CONSOLE_LOG_ERROR("connect http host " << host << ":" << port << " failure");
            return nullptr;
        }
		this->mHttpTask.Clear();
		LOG_INFO("connect http host " << host << ":" << port << " successful");
        netWorkThread.Invoke(&HttpRequestClient::Send, this, httpRequest);
        if(!this->mHttpTask.Await())
        {
			CONSOLE_LOG_ERROR("send http get request failure");
            return nullptr;
        }
		this->mHttpTask.Clear();
        std::shared_ptr<HttpAsyncResponse> httpContent(new HttpAsyncResponse(0, fs));
        netWorkThread.Invoke(&HttpRequestClient::ReceiveHttpContent, this, httpContent);
        return this->mHttpTask.Await() ? httpContent : nullptr;
#endif
    }

    void HttpRequestClient::ReceiveHttpContent(std::shared_ptr<IHttpContent> httpContent)
    {
        AsioTcpSocket & tcpSocket = this->mSocket->GetSocket();
        asio::async_read(tcpSocket, this->mReadBuffer,
                         asio::transfer_at_least(1),[this, httpContent]
            (const asio::error_code & code, size_t size)
        {
			if(code == asio::error::eof)
			{
				this->mHttpTask.SetResult(true);
				return;
			}
            else if(code)
            {
				CONSOLE_LOG_ERROR(code.message());
                this->mHttpTask.SetResult(false);
            }
			else
			{
				switch (httpContent->OnReceiveData(this->mReadBuffer))
				{
				case HttpStatus::OK:
					this->mHttpTask.SetResult(true);
					break;
				case HttpStatus::CONTINUE:
					this->ReceiveHttpContent(httpContent);
					break;
				}
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
        AsioContext & context = this->mSocket->GetThread();
        std::shared_ptr<asio::ip::tcp::resolver> resolver(new asio::ip::tcp::resolver(context));
        std::shared_ptr<asio::ip::tcp::resolver::query> query(new asio::ip::tcp::resolver::query(host, port));
        resolver->async_resolve(*query, [this, resolver, query, port, host]
            (const asio::error_code &err, asio::ip::tcp::resolver::iterator iterator)
        {
            if(err)
            {
				this->mHttpTask.SetResult(false);
                return;
            }
            AsioTcpSocket & tcpSocket = this->mSocket->GetSocket();
            asio::async_connect(tcpSocket, iterator, [this]
                (const asio::error_code & code, asio::ip::tcp::resolver::iterator iter)
            {
                if(code)
                {
					this->mHttpTask.SetResult(false);
					return;
                }
				this->mHttpTask.SetResult(true);
            });
        });
    }
}
