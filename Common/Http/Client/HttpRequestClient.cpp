//
// Created by yjz on 2022/1/19.
//
#include<regex>
#include"HttpRequestClient.h"
#include"Guid/Guid.h"
#include"Component/HttpComponent.h"
namespace Sentry
{
    HttpRequestClient::HttpRequestClient(std::shared_ptr<SocketProxy> socketProxy, HttpComponent * httpComponent)
		: Tcp::TcpContext(socketProxy)
    {
        this->mTimeout = 15; //默认十五秒
		this->mHttpComponent = httpComponent;
    }

	long long HttpRequestClient::Do(std::shared_ptr<Http::Request> httpRequest, int timeout)
	{
        this->mTimeout = timeout;
		this->mRequest = httpRequest;
        this->mTaskId = Guid::Create();
		this->mResponse = std::make_shared<Http::Response>();
#ifdef ONLY_MAIN_THREAD
		this->ConnectHost();
#else
		Asio::Context & netWorkThread = this->mSocket->GetThread();
		netWorkThread.post(std::bind(&HttpRequestClient::ConnectHost, this));
#endif
        return this->mTaskId;
	}


    void HttpRequestClient::OnConnect(const asio::error_code &error, int count)
    {
        if(error)
        {
            CONSOLE_LOG_ERROR("connect ]" << this->mRequest->Host()
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
            this->OnComplete(HttpStatus::REQUEST_TIMEOUT);
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
#ifdef ONLY_MAIN_THREAD
        this->mHttpComponent->OnResponse(taskId, std::move(this->mResponse));
#else
        Asio::Context &io = App::Inst()->MainThread();
        io.post(std::bind(&HttpComponent::OnResponse,
                          this->mHttpComponent, this->mTaskId, std::move(this->mResponse)));
#endif
    }

    void HttpRequestClient::OnReceiveLine(const Asio::Code &code, std::istream &is, size_t size)
    {
        if(this->mTimer != nullptr)
        {
            Asio::Code err;
            this->mTimer->cancel(err);
            this->mTimer = nullptr;
        }
        if(code && code != asio::error::eof)
        {
            this->OnComplete(HttpStatus::INTERNAL_SERVER_ERROR);
            return;
        }
        else
        {
            this->mResponse->OnRead(is);
            this->ReceiveSomeMessage();
        }
    }

    void HttpRequestClient::OnReceiveMessage(const asio::error_code &code, std::istream & is, size_t size)
    {
        if(code && code != asio::error::eof)
        {
            this->OnComplete(HttpStatus::INTERNAL_SERVER_ERROR);
        }
        else
        {
            if (code == asio::error::eof || size == 0)
            {
                this->OnComplete(HttpStatus::OK);
                return;
            }
            this->mResponse->OnRead(is);
            {
                this->ReceiveSomeMessage();
            }
        }
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
