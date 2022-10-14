//
// Created by yjz on 2022/1/19.
//
#include<regex>
#include"HttpRequestClient.h"
#include"Component/HttpComponent.h"
namespace Sentry
{
    HttpRequestClient::HttpRequestClient(std::shared_ptr<SocketProxy> socketProxy, HttpComponent * httpComponent)
		: Tcp::TcpContext(socketProxy)
    {
        this->mTimeout = 10;
		this->mHttpComponent = httpComponent;
    }

	void HttpRequestClient::Request(std::shared_ptr<HttpAsyncRequest> httpRequest, int time)
	{
        this->mTimeout = time;
		this->mRequest = httpRequest;
        constexpr HttpStatus defaultStatus = HttpStatus::INTERNAL_SERVER_ERROR;
		this->mResponse = std::make_shared<HttpDataResponse>(defaultStatus);
#ifdef ONLY_MAIN_THREAD
		this->ConnectHost();
#else
		Asio::Context & netWorkThread = this->mSocket->GetThread();
		netWorkThread.post(std::bind(&HttpRequestClient::ConnectHost, this));
#endif
	}

    void HttpRequestClient::Request(std::shared_ptr<HttpAsyncRequest> httpRequest, std::fstream * fs, int time)
    {
        this->mTimeout = time;
        this->mRequest = httpRequest;
        constexpr HttpStatus defaultStatus = HttpStatus::INTERNAL_SERVER_ERROR;
        this->mResponse = std::make_shared<HttpFileResponse>(fs, defaultStatus);
#ifdef ONLY_MAIN_THREAD
		this->ConnectHost();
#else
        Asio::Context & netWorkThread = this->mSocket->GetThread();
        netWorkThread.post(std::bind(&HttpRequestClient::ConnectHost, this));
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
        this->PopMessage();
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

    void HttpRequestClient::OnTimeout()
    {
        this->mSocket->Close();
        long long taskId = this->mRequest->GetTaskId();
#ifdef ONLY_MAIN_THREAD
        this->mHttpComponent->OnTimeout(taskId);
#else
        Asio::Context &io = App::Inst()->GetThread();
        io.post(std::bind(&HttpComponent::OnTimeout, this->mHttpComponent, taskId));
#endif
    }

    void HttpRequestClient::OnComplete(const asio::error_code & code)
    {
        if (code)
        {
            if(code == asio::error::operation_aborted)
            {
                return;
            }
            CONSOLE_LOG_ERROR(code.message());
            this->mResponse->SetError(code);
        }

        this->mSocket->Close();
        if (this->mTimer != nullptr)
        {
            asio::error_code err;
            this->mTimer->cancel(err);
        }
        long long taskId = this->mRequest->GetTaskId();
#ifdef ONLY_MAIN_THREAD
        this->mHttpComponent->OnResponse(taskId, std::move(this->mResponse));
#else
        Asio::Context &io = App::Inst()->GetThread();
        io.post(std::bind(&HttpComponent::OnResponse,
                          this->mHttpComponent, taskId, std::move(this->mResponse)));
#endif
        std::move(this->mRequest);
        std::move(this->mResponse);
    }

    void HttpRequestClient::OnReceiveLine(const asio::error_code &code, std::istream & is, size_t)
    {
        if(code)
        {
#ifdef __NET_ERROR_LOG__
            CONSOLE_LOG_ERROR(code.message());
#endif
            this->OnComplete(code);
            return;
        }
        switch(this->mResponse->OnReceiveLine(is))
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

    void HttpRequestClient::OnReceiveMessage(const asio::error_code &code, std::istream & is, size_t)
    {
        if(code == asio::error::eof)
        {
            asio::error_code err;
            this->OnComplete(err);
            return;
        }
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
        assert(this->mSendBuffer.size() == 0);
        assert(this->mRecvBuffer.size() == 0);
        const std::string & host = this->mRequest->GetHost();
        const std::string & port = this->mRequest->GetPort();
        Asio::Context & context = this->mSocket->GetThread();

        if(this->mTimeout > 0)
        {
            Asio::Code code{asio::error::timed_out};
            std::chrono::seconds timeout{this->mTimeout};
            this->mTimer = std::make_shared<asio::steady_timer>(context, timeout);
            this->mTimer->async_wait(std::bind(&HttpRequestClient::OnTimeout, this));
        }
        std::shared_ptr<Asio::Resolver> resolver(new Asio::Resolver (context));
        std::shared_ptr<Asio::ResolverQuery> query(new Asio::ResolverQuery(host, port));
        resolver->async_resolve(*query, [this, resolver, query, port, host]
            (const asio::error_code &err, Asio::Resolver::iterator iterator)
        {
            if(err)
            {
				this->OnConnect(err, 0);
                return;
            }
            Asio::Socket & tcpSocket = this->mSocket->GetSocket();
            asio::async_connect(tcpSocket, iterator, [this]
                (const asio::error_code & code, Asio::Resolver::iterator iter)
            {
                this->OnConnect(code, 0);
            });
        });
    }
}
