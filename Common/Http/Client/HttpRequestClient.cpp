//
// Created by yjz on 2022/1/19.
//
#include<regex>
#include <utility>
#include"HttpRequestClient.h"
#include"Util/Guid/Guid.h"
#include"Entity/App/App.h"
#include"Http/Component/HttpComponent.h"
namespace Sentry
{
    HttpRequestClient::HttpRequestClient(std::shared_ptr<SocketProxy> socketProxy, HttpComponent * httpComponent)
		: Tcp::TcpContext(std::move(socketProxy))
    {
        this->mTaskId = 0;
        this->mTimeout = 15; //默认十五秒
		this->mHttpComponent = httpComponent;
    }

	void HttpRequestClient::Do(std::shared_ptr<Http::Request> request,
		std::shared_ptr<Http::IResponse> response, int taskId, int timeout)
	{
		this->mTaskId = taskId;
		this->mTimeout = timeout;
		this->mRequest = request;
		this->mResponse = response;
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
            CONSOLE_LOG_ERROR("connect [" << this->mRequest->Host()
                 << ":" << this->mRequest->Port() << "] failure :" << error.message());
			this->OnComplete(HttpStatus::INTERNAL_SERVER_ERROR);
			return;
        }
		this->Write(this->mRequest);
    }

    void HttpRequestClient::OnSendMessage(const asio::error_code& code, std::shared_ptr<Tcp::ProtoMessage> message)
    {
        this->PopMessage();
        if (code)
        {
#ifdef __NET_ERROR_LOG__
            CONSOLE_LOG_ERROR(code.message());
#endif
            this->OnComplete(HttpStatus::INTERNAL_SERVER_ERROR);
            return;
        }
        this->ReceiveLine();
    }

    void HttpRequestClient::OnTimeout(Asio::Code code)
    {
        if(code != asio::error::operation_aborted)
        {
            this->mSocket->Close();         
			CONSOLE_LOG_ERROR("[" << this->mRequest->Method() << "] "
				<< this->mRequest->Path() << " request time out");
        }
    }

    void HttpRequestClient::OnComplete(HttpStatus status)
    {
        if(status != HttpStatus::OK)
        {
            this->mResponse->SetCode(status);
        }
        this->mSocket->Close();
        if (this->mTimer != nullptr)
        {
            asio::error_code err;
            this->mTimer->cancel_one(err);
        }
        this->mResponse->OnComplete();
#ifdef ONLY_MAIN_THREAD
        this->mHttpComponent->OnResponse(this->mTaskId, std::move(this->mResponse));
#else
        Asio::Context &io = App::Inst()->MainThread();
        io.post(std::bind(&HttpComponent::OnResponse,
                          this->mHttpComponent, this->mTaskId, std::move(this->mResponse)));
#endif
    }

    void HttpRequestClient::OnReceiveLine(const Asio::Code &code, std::istream &is, size_t size)
	{
		if (this->mTimer != nullptr)
		{
			Asio::Code err;
			this->mTimer->cancel(err);
			this->mTimer = nullptr;
		}
		if (code && code != asio::error::eof)
		{
			this->OnReadLater(HTTP_READ_ERROR);
			return;
		}
		this->OnReadLater(this->mResponse->OnRead(is));
	}

	void HttpRequestClient::OnReadLater(int num)
	{
		switch(num)
		{
			case 0: //完成
				this->OnComplete(HttpStatus::OK);
				break;
			case -1: //读一行
				this->ReceiveLine();
				break;
			case -2: //读一些
				this->ReceiveSomeMessage();
				break;
			case -3: //消息错误
				this->OnComplete(HttpStatus::INTERNAL_SERVER_ERROR);
				break;
			default:
				this->ReceiveMessage(num);
				break;
		}
	}

    void HttpRequestClient::OnReceiveMessage(const asio::error_code &code, std::istream & is, size_t size)
	{
		if (code && code != asio::error::eof)
		{
			this->OnReadLater(HTTP_READ_ERROR);
			return;
		}
		if (code == asio::error::eof || size == 0)
		{
			this->OnComplete(HttpStatus::OK);
			this->OnReadLater(HTTP_READ_COMPLETE);
			return;
		}
		this->OnReadLater(this->mResponse->OnRead(is));
	}

    void HttpRequestClient::ConnectHost()
    {
        assert(this->mSendBuffer.size() == 0);
        assert(this->mRecvBuffer.size() == 0);
        const std::string & host = this->mRequest->Host();
        const std::string & port = this->mRequest->Port();
        Asio::Context & context = this->mSocket->GetThread();

        if(this->mTimeout > 0)
        {
            std::chrono::seconds timeout{this->mTimeout};
            this->mTimer = std::make_shared<asio::steady_timer>(context, timeout);
            this->mTimer->async_wait(std::bind(&HttpRequestClient::OnTimeout, this, args1));
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
