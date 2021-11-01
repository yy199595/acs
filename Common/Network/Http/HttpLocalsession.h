//
// Created by zmhy0073 on 2021/10/29.
//

#ifndef SENTRY_HTTPLOCALSESSION_H
#define SENTRY_HTTPLOCALSESSION_H
#include <Network/Http/HttpHandlerBase.h>
#include <Network/Http/HttpSessionBase.h>
namespace Sentry
{
	class HttpClientComponent;
	class HttpLocalSession : public HttpSessionBase
	{
	public:
		HttpLocalSession(HttpClientComponent * component, HttpHandlerBase * handler);
		~HttpLocalSession() override;
	public:	
		SocketType GetSocketType() override { return SocketType::LocalSocket; }
		void StartConnectHost(const std::string & host, const std::string & port);
	protected:
        void OnSendHttpMessageAfter() override;
        bool WriterToBuffer(std::ostream & os) override;
        void OnSocketError(const asio::error_code &err) override;
        bool OnReceiveBody(asio::streambuf & buf, const asio::error_code & code) override;
		bool OnReceiveHeard(asio::streambuf & buf,size_t  size, const asio::error_code & code) override;

	private:
		void Resolver();
		void ConnectHandler(const asio::error_code & err);
	private:
		std::string mHost;
		std::string mPort;
		HttpHandlerBase * mHttpHandler;
		HttpClientComponent * mComponent;
		asio::ip::tcp::resolver * mResolver;
		asio::ip::tcp::resolver::query * mQuery;
	};
}
#endif //SENTRY_HTTPLOCALSESSION_H
