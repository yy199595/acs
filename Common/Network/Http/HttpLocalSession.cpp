#include "HttpLocalSession.h"
#include "HttpRequest.h"
#include <Component/IComponent.h>
namespace Sentry
{
	HttpLocalSession::HttpLocalSession(ISocketHandler * handler, HttpRequest & request)
		: HttpSessionBase(handler), mHttpRequest(request)
	{
		this->mQuery = nullptr;
		this->mResolver = nullptr;
	}

	HttpLocalSession::~HttpLocalSession()
	{
		if (this->mQuery)
		{
			delete this->mQuery;
		}
		if (this->mResolver)
		{
			delete this->mResolver;
		}
	}


	void HttpLocalSession::OnConnect(const asio::error_code &err)
	{
        if(err)
        {
            SayNoDebugError(err.message());
            this->mHttpRequest.OnReceiveDone(true);
        }
        else
        {
            this->mHttpRequest.GetRquestParame(this->mStream);
            this->SendByStream(&mStream);
        }
	}

	bool HttpLocalSession::OnReceive(asio::streambuf & stream, const asio::error_code & err)
	{
		return this->mHttpRequest.OnReceive(stream, err);
	}

	void HttpLocalSession::OnSendByStream(asio::streambuf *msg, const asio::error_code &err)
	{
        if(err)
        {
            SayNoDebugError(err.message());
            this->mHttpRequest.OnReceiveDone(true);
        }
        else
        {
            this->StartReceive();
        }
	}

	void HttpLocalSession::ConnectHostHandler(const std::string & host, unsigned short port)
	{		
		this->mResolver = new asio::ip::tcp::resolver(this->GetContext());		
		this->mQuery = new asio::ip::tcp::resolver::query(host, std::to_string(port));
		this->mResolver->async_resolve(*mQuery, [this, host, port]
			(const asio::error_code & err, asio::ip::tcp::resolver::iterator iterator)
		{
			asio::ip::tcp::resolver::iterator end;
			for (auto iter = iterator; iter != end; iter++)
			{
				const asio::ip::tcp::endpoint & point = (*iter).endpoint();
				SayNoDebugInfo(host << " = " << point.address().to_string() << ":" << point.port());
			}
            if(err)
            {
                SayNoDebugError(err.message());
                this->mHttpRequest.OnReceiveDone(true);
            }
            else
            {
                asio::async_connect(this->GetSocket(), iterator,
                                    std::bind(&HttpLocalSession::OnConnect, this, args1));
            }
		});
	}

	bool HttpLocalSession::StartConnectHost(const std::string & name, const std::string & host, unsigned short port)
	{
		if (this->GetSocketType() == RemoteSocket)
		{
			return false;
		}
		if (this->IsActive())
		{
			return true;
		}
		this->mHandler->GetNetThread()->AddTask(&HttpLocalSession::ConnectHostHandler, this, host, port);
		return true;
	}
}
