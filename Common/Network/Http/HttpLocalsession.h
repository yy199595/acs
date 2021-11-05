//
// Created by zmhy0073 on 2021/10/29.
//

#ifndef GameKeeper_HTTPLOCALSESSION_H
#define GameKeeper_HTTPLOCALSESSION_H
#include <Network/Http/HttpHandlerBase.h>
#include <Network/Http/HttpSessionBase.h>
namespace GameKeeper
{
	class HttpClientComponent;
	class HttpLocalSession : public HttpSessionBase
	{
	public:
		HttpLocalSession(HttpClientComponent * component, HttpHandlerBase * handler);
		~HttpLocalSession() override;
	public:
        void SetSocketProxy(SocketProxy * socketProxy) override { }
		void StartConnectHost(const std::string & host, const std::string & port, SocketProxy * socketProxy);
	protected:
        void OnWriteAfter(XCode code) override;
        void OnReceiveBodyAfter(XCode code) override;
        void OnReceiveHeardAfter(XCode code) override;
        bool WriterToBuffer(std::ostream & os) override;
        void OnReceiveBody(asio::streambuf & buf) override;
		bool OnReceiveHeard(asio::streambuf & buf,size_t  size) override;

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
#endif //GameKeeper_HTTPLOCALSESSION_H
