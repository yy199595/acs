//
// Created by zmhy0073 on 2021/10/29.
//

#ifndef GameKeeper_HTTPLOCALSESSION_H
#define GameKeeper_HTTPLOCALSESSION_H
#include <Network/Http/HttpSessionBase.h>
namespace GameKeeper
{

	class HttpRequest;
    class HttpReadContent;
    class HttpWriteContent;
    class HttpGetRequest;
    class HttpPostRequest;
	class HttpHandlerBase;
	class HttpClientComponent;
	class HttpLocalSession : public HttpSessionBase
	{
	public:
		explicit HttpLocalSession(HttpClientComponent * component);
		~HttpLocalSession() final;
	public:
		void Clear() override;
		SocketType GetSocketType() final { return SocketType::LocalSocket; }

    public:
        XCode Get(const std::string & url, HttpReadContent & response);
        XCode Post(const std::string & url, HttpWriteContent & request, HttpReadContent & response);
	protected:
		HttpHandlerBase * GetHandler() final;

        void OnWriterAfter(XCode code) final;

        void OnReceiveHeardAfter(XCode code) final;

		bool OnReceiveHeard(asio::streambuf & buf) final;

    private:
        void StartReceiveBody();

	private:
		void Resolver();
        void SetCode(XCode code);
        void ConnectHandler(const asio::error_code & err);
        void ReadBodyCallback(const asio::error_code & err, size_t size);
        void StartConnectHost(const std::string & host, const std::string & port);
    private:
        HttpRequest * mHttpHandler;
        HttpGetRequest * mGetHandler;
        HttpPostRequest * mPostHandler;
    private:
		std::string mHost;
		std::string mPort;
        unsigned int mCorId;
		HttpClientComponent * mComponent;
		asio::ip::tcp::resolver * mResolver;
		asio::ip::tcp::resolver::query * mQuery;
	};
}
#endif //GameKeeper_HTTPLOCALSESSION_H
