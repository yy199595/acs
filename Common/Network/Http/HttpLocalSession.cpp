//
// Created by zmhy0073 on 2021/10/29.
//

#include "HttpLocalsession.h"
#include <Network/Http/HttpClientComponent.h>
#include<Network/Http/Request/HttpGetRequest.h>
#include<Network/Http/Request/HttpPostRequest.h>

#include <Core/App.h>
#include <Network/NetworkHelper.h>
#include <Scene/TaskPoolComponent.h>
#include <Network/Http/Content/HttpReadContent.h>

namespace GameKeeper
{
	HttpLocalSession::HttpLocalSession(HttpClientComponent * component)
		: HttpSessionBase(component)
	{
        this->mCorId = 0;
		this->mQuery = nullptr;
		this->mResolver = nullptr;
        this->mHttpHandler = nullptr;
        this->mGetHandler = new HttpGetRequest(component);
        this->mPostHandler = new HttpPostRequest(component);
	}

	HttpLocalSession::~HttpLocalSession()
    {		
        delete this->mQuery;
        delete this->mResolver;
        delete this->mGetHandler;
        delete this->mPostHandler;
    }

	void HttpLocalSession::StartConnectHost(const std::string & host, const std::string & port)
    {
        this->mHost = host;
        this->mPort = port;
        delete this->mSocketProxy;
        auto taskComponent = App::Get().GetComponent<TaskPoolComponent>();
        NetWorkThread &netWorkThread = taskComponent->GetNetThread();
        this->mSocketProxy = new SocketProxy(netWorkThread, "HttpGet");

        if (netWorkThread.IsCurrentThread())
        {
            this->Resolver();
            return;
        }
        netWorkThread.AddTask(&HttpLocalSession::Resolver, this);
    }

	HttpHandlerBase * HttpLocalSession::GetHandler()
	{
		return this->mHttpHandler;
	}

	bool HttpLocalSession::OnReceiveHeard(asio::streambuf & buf)
    {
        GKAssertRetFalse_F(this->mHttpHandler);
        return this->mHttpHandler->OnReceiveHeard(buf);
    }

	void HttpLocalSession::Resolver()
    {
        delete this->mQuery;
        delete this->mResolver;
		asio::io_context &ctx = mSocketProxy->GetContext();
        this->mResolver = new asio::ip::tcp::resolver(ctx);
        this->mQuery = new asio::ip::tcp::resolver::query(this->mHost, this->mPort);
        this->mResolver->async_resolve(*this->mQuery, [this](
                const asio::error_code &err, asio::ip::tcp::resolver::iterator iterator)
        {
            if (err)
            {
                this->SetCode(XCode::HttpNetWorkError);
                GKDebugError("resolver " << this->mHost << ":" << this->mPort << " failure : " << err.message());
                return;
            }
			AsioTcpSocket & socket = this->mSocketProxy->GetSocket();
			asio::async_connect(socket, std::move(iterator),
				std::bind(&HttpLocalSession::ConnectHandler, this, args1));
        });
    }

    XCode HttpLocalSession::Get(const std::string &url, HttpReadContent &response)
    {
        if(!this->mGetHandler->Init(url, response))
        {
            return XCode::HttpUrlParseError;
        }
        this->mHttpHandler = this->mGetHandler;
        this->StartConnectHost(this->mGetHandler->GetHost(), this->mGetHandler->GetPort());

        App::Get().GetCorComponent()->YieldReturn(this->mCorId);
        if(this->mCode != XCode::Successful)
        {
            return this->mCode;
        }
        if(this->mHttpHandler->GetHttpCode() != HttpStatus::OK)
        {
            return XCode::HttpResponseError;
        }
        return XCode::Successful;
    }

    XCode HttpLocalSession::Post(const std::string &url, HttpWriteContent &request, HttpReadContent &response)
    {
        if(!this->mPostHandler->Init(url, request, response))
        {
            return XCode::HttpUrlParseError;
        }
        this->mHttpHandler = this->mPostHandler;
        this->StartConnectHost(this->mPostHandler->GetHost(), this->mPostHandler->GetPort());

        App::Get().GetCorComponent()->YieldReturn(this->mCorId);
        if(this->mCode != XCode::Successful)
        {
            return this->mCode;
        }
        if(this->mHttpHandler->GetHttpCode() != HttpStatus::OK)
        {
            return XCode::HttpResponseError;
        }
        return XCode::Successful;
    }

    void HttpLocalSession::OnWriterAfter(XCode code)
    {
        if (code != XCode::Successful)
        {
            this->SetCode(code);
            return;
        }
        this->StartReceiveHeard();
    }

    void HttpLocalSession::OnReceiveHeardAfter(XCode code)
    {
        if(code != XCode::Successful)
        {
            this->SetCode(code);
            return;
        }
        this->StartReceiveBody();
    }

    void HttpLocalSession::SetCode(XCode code)
    {
        this->mCode = code;
        CoroutineComponent *corComponent = App::Get().GetCorComponent();
        MainTaskScheduler &taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.AddMainTask(&CoroutineComponent::Resume, corComponent, this->mCorId);
    }

    void HttpLocalSession::Clear()
    {
        HttpSessionBase::Clear();

        this->mCorId = 0;
        this->mHost.clear();
        this->mPort.clear();


        this->mGetHandler->Clear();
        this->mPostHandler->Clear();
        this->mHttpHandler = nullptr;

        delete this->mQuery;
        delete this->mResolver;
        this->mResolver = nullptr;
        this->mQuery = nullptr;
    }


	void HttpLocalSession::ConnectHandler(const asio::error_code & err)
    {
        if (err)
        {
            this->SetCode(XCode::HttpNetWorkError);
            GKDebugError("connect " << this->mHost << ":" << this->mPort << " failure : " << err.message());
            return;
        }
		AsioTcpSocket & socket = this->mSocketProxy->GetSocket();
        this->mAddress = socket.remote_endpoint().address().to_string()
                         + ":" + std::to_string(socket.remote_endpoint().port());
        GKDebugLog("connect to " << this->mHost << ":" << this->mPort << " successful");
        this->StartSendHttpMessage();
    }

    void HttpLocalSession::StartReceiveBody()
    {
        asio::error_code code;
        AsioTcpSocket &socket = this->mSocketProxy->GetSocket();
        if(this->mStreamBuf.size() > 0)
        {
            this->mHttpHandler->OnReceiveBody(this->mStreamBuf);
        }
        if(!this->mSocketProxy->IsOpen())
        {
            this->SetCode(XCode::NetReceiveFailure);
        }
        else
        {
            asio::async_read(socket, this->mStreamBuf, asio::transfer_at_least(1),
                             std::bind(&HttpLocalSession::ReadBodyCallback, this, args1, args2));
        }
    }

    void HttpLocalSession::ReadBodyCallback(const asio::error_code &err, size_t size)
    {
        GKAssertRet_F(this->mHttpHandler);
        if(err == asio::error::eof)
        {
            this->SetCode(XCode::Successful);
        }
        else if(err)
        {
            this->SetCode(XCode::NetReceiveFailure);
        }
        else
        {
            this->mHttpHandler->OnReceiveBody(this->mStreamBuf);
            AsioContext & context = this->mSocketProxy->GetContext();
            context.post(std::bind(&HttpLocalSession::StartReceiveBody, this));
        }
    }
}