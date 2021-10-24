#pragma once
#include<string>
#include<XCode/XCode.h>
#include<Define/CommonTypeDef.h>
#include<Network/Http/HttpResponse.h>
namespace Sentry
{
	class ISocketHandler;
	class MainTaskScheduler;
	class CoroutineComponent;
	class HttpLocalSession;
	class HttpRequest
	{
	public:
		HttpRequest(ISocketHandler * handler, const std::string & name);
		virtual ~HttpRequest() { }
	public:
		bool Connect(const std::string & host, unsigned short port);
		virtual void GetRquestParame(asio::streambuf & strem) = 0;
		virtual bool OnReceive(asio::streambuf & strem, const asio::error_code & err) = 0;
		
	public:
		unsigned short GetPort() { this->mPort; }
		const std::string & GetName() { return this->mName; }
		const std::string & GetHost() { return this->mHost; }
		
	protected:
		std::string mHost;
		unsigned short mPort;
		const std::string mName;
		HttpLocalSession * mHttpSession;
	};
}

namespace Sentry
{
	
	class HttpGetRequest : public HttpRequest
	{
	public:
		HttpGetRequest(ISocketHandler * handler);
	public:
		XCode Get(const std::string & url, std::string & response);
	public:	
		void GetRquestParame(asio::streambuf & strem) override;
		bool OnReceive(asio::streambuf & strem, const asio::error_code & err) override;
	private:
		XCode mCode;
		std::string mPath;
		std::string mName;
		HttpResponse mResponse;
		unsigned int mCoroutineId;
		MainTaskScheduler & mMainTaskPool;
		CoroutineComponent * mCorComponent;
	};
}

namespace Sentry
{
	class HttpPostRequest : public HttpRequest
	{
	public:

	};
}