//
// Created by zmhy0073 on 2021/10/29.
//

#include "HttpLocalsession.h"
#include <Network/Http/HttpClientComponent.h>

#include <utility>
namespace GameKeeper
{
	HttpLocalSession::HttpLocalSession(HttpClientComponent * component, HttpHandlerBase * handler)
		: HttpSessionBase(component, "LocalHttpSession"), mHttpHandler(handler)
	{
		this->mQuery = nullptr;
		this->mResolver = nullptr;
	}

	HttpLocalSession::~HttpLocalSession()
    {
        delete this->mQuery;
        delete this->mResolver;
    }

	void HttpLocalSession::StartConnectHost(const std::string & host, const std::string & port)
	{
		this->mHost = host;
		this->mPort = port;
		this->mHandler.GetNetThread()->AddTask(&HttpLocalSession::Resolver, this);
	}

	bool HttpLocalSession::WriterToBuffer(std::ostream & os)
	{
        GKAssertRetFalse_F(this->mHttpHandler);
        return this->mHttpHandler->WriterToBuffer(os);
	}

	void HttpLocalSession::OnReceiveBody(asio::streambuf & buf)
	{
        GKAssertRet_F(this->mHttpHandler);
        this->mHttpHandler->OnReceiveBody(buf);
	}

	bool HttpLocalSession::OnReceiveHeard(asio::streambuf & buf, size_t size)
    {
        GKAssertRetFalse_F(this->mHttpHandler);
        return this->mHttpHandler->OnReceiveHeard(buf, size);
    }

	void HttpLocalSession::Resolver()
    {
        asio::io_context &ctx = this->mHandler.GetContext();
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
			asio::async_connect(this->GetSocket(), std::move(iterator),
				std::bind(&HttpLocalSession::ConnectHandler, this, args1));
        });
    }

    void HttpLocalSession::OnReceiveBodyAfter(XCode code)
    {
        GKAssertRet_F(this->mHttpHandler);
        this->mHttpHandler->OnReceiveBodyAfter(code);
    }

    void HttpLocalSession::OnReceiveHeardAfter(XCode code)
    {
        GKAssertRet_F(this->mHttpHandler);
        this->mHttpHandler->OnReceiveHeardAfter(code);
    }

	void HttpLocalSession::ConnectHandler(const asio::error_code & err)
    {
        if (err)
        {
            this->mHttpHandler->OnReceiveHeardAfter(XCode::HttpNetWorkError);
            GKDebugError("connect " << this->mHost << ":" << this->mPort << " failure : " << err.message());
            return;
        }
        this->mAddress = this->mSocket->remote_endpoint().address().to_string()
                         + ":" + std::to_string(this->mSocket->remote_endpoint().port());
        GKDebugLog("connect to " << this->mHost << ":" << this->mPort << " successful");
        this->StartSendHttpMessage();
    }

    void HttpLocalSession::OnWriteAfter(XCode code)
    {
        GKAssertRet_F(this->mHttpHandler);
        if(code == XCode::Successful)
        {
            this->StartReceiveHeard();
        }
        this->mHttpHandler->OnWriterAfter(code);
    }
}