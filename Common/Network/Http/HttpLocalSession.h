#pragma once
#include <XCode/XCode.h>
#include "HttpSessionBase.h"
namespace Sentry
{
	class HttpRequest;
	class HttpLocalSession : public HttpSessionBase
	{
	public:
		HttpLocalSession(ISocketHandler * handler, HttpRequest & request);
		~HttpLocalSession();
	protected:
		void OnConnect(const asio::error_code &err) override;		
		void OnSendByStream(asio::streambuf *msg, const asio::error_code &err) override;
		bool OnReceive(asio::streambuf & stream, const asio::error_code & err) override;
	private:
		void ConnectHostHandler(const std::string & host, unsigned short port);		
	public:
		bool StartConnectHost(const std::string & name, const std::string & host, unsigned short port);
	private:
		asio::ip::tcp::resolver * mResolver;
		asio::ip::tcp::resolver::query * mQuery;
	private:
		std::string mHost;
		std::string mPath;
		unsigned short mPort;
		asio::streambuf mStream;
		unsigned int mCoroutineId;
		HttpRequest & mHttpRequest;	
	};
}