//
// Created by yjz on 2022/1/19.
//

#ifndef GAMEKEEPER_HTTPREQUESTCLIENT_H
#define GAMEKEEPER_HTTPREQUESTCLIENT_H
#include"Http.h"
#include"Tcp/TcpContext.h"
#include"Http/HttpRequest.h"
#include"Http/HttpResponse.h"
namespace Sentry
{
	class HttpComponent;
	class HttpRequestClient : public Tcp::TcpContext
	{
	 public:
		HttpRequestClient(std::shared_ptr<SocketProxy> socketProxy, HttpComponent * component);
	 public:
		long long Do(std::shared_ptr<Http::Request> request, int timeout = 0);
	 private:
        void ConnectHost();
        void OnTimeout(Asio::Code code);
        void OnComplete(HttpStatus code);
        void OnConnect(const asio::error_code &error, int count) final;
        void OnReceiveLine(const Asio::Code &code, std::istream &is, size_t size) final;
        void OnReceiveMessage(const asio::error_code &code, std::istream & is, size_t) final;
		void OnSendMessage(const asio::error_code &code, std::shared_ptr<Tcp::ProtoMessage> message) final;
	 private:
        int mTimeout;
        long long mTaskId;
		HttpComponent * mHttpComponent;
        std::shared_ptr<Http::Request> mRequest;
		std::shared_ptr<Http::Response> mResponse;
        std::shared_ptr<asio::steady_timer> mTimer;
    };
}
#endif //GAMEKEEPER_HTTPREQUESTCLIENT_H
