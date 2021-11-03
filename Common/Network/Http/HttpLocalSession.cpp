//
// Created by zmhy0073 on 2021/10/29.
//

#include "HttpLocalsession.h"
#include <Network/Http/HttpClientComponent.h>
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
		if (this->mHttpHandler == nullptr)
		{
			return true;
		}
		return this->mHttpHandler->WriterToBuffer(os);
	}

	void HttpLocalSession::OnReceiveBody(asio::streambuf & buf)
	{
		if (this->mHttpHandler == nullptr)
		{
			return;
		}
        this->mHttpHandler->OnReceiveBody(buf);
	}

	bool HttpLocalSession::OnReceiveHeard(asio::streambuf & buf, size_t size)
    {
        if (this->mHttpHandler == nullptr)
        {
            return false;
        }
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
                this->mHttpHandler->OnSessionError(err);
                return;
            }
            asio::ip::tcp::resolver::iterator end;
            for (auto iter = iterator; iter != end; iter++)
            {
                GKDebugInfo(this->mHost << " : "
                                           << (*iter).endpoint().address().to_string());
            }
			asio::async_connect(this->GetSocket(), iterator,
				std::bind(&HttpLocalSession::ConnectHandler, this, args1));
        });
    }

    void HttpLocalSession::OnSocketError(const asio::error_code &err)
    {
        if (this->mHttpHandler == nullptr)
        {
            return;
        }
        this->mHttpHandler->OnSessionError(err);
    }

	void HttpLocalSession::ConnectHandler(const asio::error_code & err)
    {
        if (err)
        {
            this->mHttpHandler->OnSessionError(err);
            return;
        }
        this->StartSendHttpMessage();
    }

    void HttpLocalSession::OnWriteAfter()
    {
        this->StartReceiveHeard();
    }

}