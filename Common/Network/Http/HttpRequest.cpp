#include "HttpRequest.h"
#include<Core/App.h>
#include<Define/CommonDef.h>
#include<Network/Http/HttpLocalSession.h>
namespace Sentry
{
	HttpRequest::HttpRequest(ISocketHandler * handler, const std::string & name)
		:mHttpSession(nullptr), mName(name)
	{
		this->mHttpSession = new HttpLocalSession(handler, *this);
	}

	bool HttpRequest::Connect(const std::string & host, unsigned short port)
	{
		if (this->mHttpSession == nullptr)
		{
			return false;
		}
		this->mHost = host;
		this->mPort = port;
		this->mHttpSession->StartConnectHost(this->mName, host, port);
		return true;
	}

	void HttpGetRequest::GetRquestParame(asio::streambuf & strem)
	{
		std::ostream os(&strem);
		os << "GET " << mPath << " HTTP/1.0\r\n";
		os << "Host: " << mHost << "\r\n";
		os << "Accept: */*\r\n";
		os << "Connection: close\r\n\r\n";
	}

	bool HttpGetRequest::OnReceive(asio::streambuf & strem, const asio::error_code & err)
	{
		if (err == asio::error::eof)
		{
			this->mCode = XCode::Successful;
			this->mResponse.ParseHttpResponse(strem);
			this->mMainTaskPool.AddMainTask(&CoroutineComponent::Resume, this->mCorComponent, this->mCoroutineId);
			return false;
		}
		else if (err)
		{
			SayNoDebugError(err.message());
			this->mCode = XCode::HttpNetWorkError;		
			this->mMainTaskPool.AddMainTask(&CoroutineComponent::Resume, this->mCorComponent, this->mCoroutineId);
			return false;
		}
		/*else
		{
			std::istreambuf_iterator<char> eos;
			std::istream response_stream(&strem);
			this->mResponse->append(std::istreambuf_iterator<char>(response_stream), eos);
		}*/
		return true;
	}

	HttpGetRequest::HttpGetRequest(ISocketHandler * handler)
		: HttpRequest(handler, "HttpGetRequest"),
		mMainTaskPool(App::Get().GetTaskScheduler())
	{
		this->mCorComponent = App::Get().GetCoroutineComponent();
	}

	XCode HttpGetRequest::Get(const std::string & url, std::string & response)
	{
		if (!NetworkHelper::ParseHttpUrl(url, this->mHost, this->mPort, this->mPath))
		{
			return XCode::HttpUrlParseError;
		}
		if (!this->Connect(this->mHost, this->mPort))
		{
			return XCode::HttpNetWorkError;
		}
		this->mCorComponent->YieldReturn(this->mCoroutineId);
		if (this->mCode != XCode::Successful)
		{
			return this->mCode;
		}
		return this->mResponse.GetResponse(response);
	}
	
}
