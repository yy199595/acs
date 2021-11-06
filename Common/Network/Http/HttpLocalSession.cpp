//
// Created by zmhy0073 on 2021/10/29.
//

#include "HttpLocalsession.h"
#include <Network/Http/HttpClientComponent.h>
#include<Network/Http/Request/HttpRequest.h>
#include <utility>
namespace GameKeeper
{
	HttpLocalSession::HttpLocalSession(HttpClientComponent * component, HttpRequest * handler)
		: HttpSessionBase(component), mHttpHandler(handler)
	{
		this->mQuery = nullptr;
		this->mResolver = nullptr;
	}

	HttpLocalSession::~HttpLocalSession()
    {		
        delete this->mQuery;
        delete this->mResolver;
    }

	void HttpLocalSession::StartConnectHost(const std::string & host, const std::string & port, SocketProxy * socketProxy)
	{
		this->mHost = host;
		this->mPort = port;
		this->mSocketProxy = socketProxy;
		NetWorkThread & nThread = this->mSocketProxy->GetThread();
		if (nThread.IsCurrentThread())
		{
			this->Resolver();
			return;
		}
		nThread.AddTask(&HttpLocalSession::Resolver, this);
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
		asio::io_context &ctx = this->mSocketProxy->GetContext();
        this->mResolver = new asio::ip::tcp::resolver(ctx);
        this->mQuery = new asio::ip::tcp::resolver::query(this->mHost, this->mPort);
        this->mResolver->async_resolve(*this->mQuery, [this](
                const asio::error_code &err, asio::ip::tcp::resolver::iterator iterator)
        {
            if (err)
            {
                this->mHttpHandler->OnReceiveHeardAfter(XCode::HttpNetWorkError);
                GKDebugError("resolver " << this->mHost << ":" << this->mPort << " failure : " << err.message());
                return;
            }
			AsioTcpSocket & socket = this->mSocketProxy->GetSocket();
			asio::async_connect(socket, std::move(iterator),
				std::bind(&HttpLocalSession::ConnectHandler, this, args1));
        });
    }


	void HttpLocalSession::ConnectHandler(const asio::error_code & err)
    {
        if (err)
        {
            this->mHttpHandler->OnReceiveHeardAfter(XCode::HttpNetWorkError);
            GKDebugError("connect " << this->mHost << ":" << this->mPort << " failure : " << err.message());
            return;
        }
		AsioTcpSocket & socket = this->mSocketProxy->GetSocket();
        this->mAddress = socket.remote_endpoint().address().to_string()
                         + ":" + std::to_string(socket.remote_endpoint().port());
        GKDebugLog("connect to " << this->mHost << ":" << this->mPort << " successful");
        this->StartSendHttpMessage();
    }
}