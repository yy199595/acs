//
// Created by zmhy0073 on 2021/10/29.
//

#ifndef GameKeeper_HTTPLOCALSESSION_H
#define GameKeeper_HTTPLOCALSESSION_H
#include <Network/Http/HttpSessionBase.h>
namespace GameKeeper
{
	class HttpRequest;
	class HttpHandlerBase;
	class HttpClientComponent;
	class HttpLocalSession : public HttpSessionBase
	{
	public:
		HttpLocalSession(HttpClientComponent * component, HttpRequest * handler);
		~HttpLocalSession() final;
	public:
		void SetSocketProxy(SocketProxy * socketProxy) { };
		SocketType GetSocketType() final { return SocketType::LocalSocket; }
		void StartConnectHost(const std::string & host, const std::string & port, SocketProxy * socketProxy);
	protected:    
		HttpHandlerBase * GetHandler() final;
		bool OnReceiveHeard(asio::streambuf & buf) final;
		
	private:
		void Resolver();
		void ConnectHandler(const asio::error_code & err);
	private:
		std::string mHost;
		std::string mPort;
		HttpRequest * mHttpHandler;
		HttpClientComponent * mComponent;
		asio::ip::tcp::resolver * mResolver;
		asio::ip::tcp::resolver::query * mQuery;
	};
}
#endif //GameKeeper_HTTPLOCALSESSION_H
