//
// Created by yjz on 2022/1/19.
//
#include<regex>
#include"HttpRequestClient.h"
#include"Component/Http/HttpComponent.h"
namespace Sentry
{
    HttpRequestClient::HttpRequestClient(std::shared_ptr<SocketProxy> socketProxy, HttpComponent * httpComponent)
		: Tcp::TcpContext(socketProxy)
    {
		this->mHttpComponent = httpComponent;
    }

	void HttpRequestClient::Request(std::shared_ptr<HttpAsyncRequest> httpRequest)
	{
		this->mRequest = httpRequest;
		this->mResponse = std::make_shared<HttpDataResponse>();
#ifdef ONLY_MAIN_THREAD
		this->ConnectHost();
#else
		IAsioThread & netWorkThread = this->mSocket->GetThread();
		netWorkThread.Invoke(&HttpRequestClient::ConnectHost, this);
#endif
	}

    void HttpRequestClient::Request(std::shared_ptr<HttpAsyncRequest> httpRequest, std::fstream * fs)
    {
        this->mRequest = httpRequest;
        this->mResponse = std::make_shared<HttpFileResponse>(fs);
#ifdef ONLY_MAIN_THREAD
		this->ConnectHost();
#else
        IAsioThread & netWorkThread = this->mSocket->GetThread();
        netWorkThread.Invoke(&HttpRequestClient::ConnectHost, this);
#endif
    }

    void HttpRequestClient::OnConnect(const asio::error_code &error, int count)
    {
        if(error)
        {
            CONSOLE_LOG_ERROR("connect ]" << this->mRequest->GetHost()
                 << ":" << this->mRequest->GetPort() << "] failure :" << error.message());
			this->OnComplete(error);
			return;
        }
        this->Send(this->mRequest);
    }

    void HttpRequestClient::OnSendMessage(const asio::error_code& code, std::shared_ptr<Tcp::ProtoMessage> message)
    {
        if (code)
        {
#ifdef __NET_ERROR_LOG__
            CONSOLE_LOG_ERROR(code.message());
#endif
            this->OnComplete(code);
            return;
        }
        this->ReceiveLine();
    }

    void HttpRequestClient::OnComplete(const asio::error_code & code)
    {
        if(code)
        {
            this->mResponse->SetError(code);
        }
        long long taskId = this->mRequest->GetTaskId();
#ifdef ONLY_MAIN_THREAD
        this->mHttpComponent->OnResponse(taskId, this->mResponse);
#else
        IAsioThread & netWorkThread = this->mSocket->GetThread();
        netWorkThread.Invoke(&HttpComponent::OnResponse, this->mHttpComponent, taskId, this->mResponse);
#endif
        this->mRequest = nullptr;
        this->mResponse = nullptr;
    }

    void HttpRequestClient::OnReceiveLine(const asio::error_code &code, size_t size)
    {
        if(code || size == 0)
        {
#ifdef __NET_ERROR_LOG__
            CONSOLE_LOG_ERROR(code.message());
#endif
            this->OnComplete(code);
            return;
        }
		std::istream & os = this->GetReadStream();
        switch(this->mResponse->OnReceiveLine(os))
        {
            case -1: //读一行
                this->ReceiveLine();
                break;
            case 1: //继续读
                this->ReceiveSomeMessage();
                break;
            case 0: //完成
                asio::error_code err;
                this->OnComplete(err);
                break;
        }
    }

    void HttpRequestClient::OnReceiveMessage(const asio::error_code &code, size_t size)
    {
        if(code == asio::error::eof)
        {
            asio::error_code err;
            this->OnComplete(err);
            return;
        }
		std::istream & is = this->GetReadStream();
        switch(this->mResponse->OnReceiveSome(is))
        {
            case 1: //继续读
                this->ReceiveSomeMessage();
                break;
            case 0: //完成
                asio::error_code err;
                this->OnComplete(err);
                break;
        }
    }

    void HttpRequestClient::ConnectHost()
    {
        AsioContext & context = this->mSocket->GetThread();
        const std::string & host = this->mRequest->GetHost();
        const std::string & port = this->mRequest->GetPort();
        std::shared_ptr<asio::ip::tcp::resolver> resolver(new asio::ip::tcp::resolver(context));
        std::shared_ptr<asio::ip::tcp::resolver::query> query(new asio::ip::tcp::resolver::query(host, port));
        resolver->async_resolve(*query, [this, resolver, query, port, host]
            (const asio::error_code &err, asio::ip::tcp::resolver::iterator iterator)
        {
            if(err)
            {
				this->OnConnect(err, 0);
                return;
            }
            AsioTcpSocket & tcpSocket = this->mSocket->GetSocket();
            asio::async_connect(tcpSocket, iterator, [this]
                (const asio::error_code & code, asio::ip::tcp::resolver::iterator iter)
            {
                this->OnConnect(code, 0);
            });
        });
    }
}
