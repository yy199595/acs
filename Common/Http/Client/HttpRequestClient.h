//
// Created by yjz on 2022/1/19.
//

#ifndef GAMEKEEPER_HTTPREQUESTCLIENT_H
#define GAMEKEEPER_HTTPREQUESTCLIENT_H
#include"Source/TaskSource.h"
#include"HttpAsyncRequest.h"
#include"Tcp/TcpContext.h"
namespace Sentry
{
	class HttpComponent;
	class HttpRequestClient : public Tcp::TcpContext
	{
	 public:
		HttpRequestClient(std::shared_ptr<SocketProxy> socketProxy, HttpComponent * component);
	 public:
		void Request(std::shared_ptr<HttpAsyncRequest> request, int time = 15);
		void Request(std::shared_ptr<HttpAsyncRequest> request, std::fstream * fs, int time = 15);
	 private:
        void OnTimeout();
        void ConnectHost();
        void OnComplete(const asio::error_code & code);
        void OnConnect(const asio::error_code &error, int count) final;
        void OnReceiveLine(const asio::error_code &code, std::istream & is, size_t) final;
        void OnReceiveMessage(const asio::error_code &code, std::istream & is, size_t) final;
		void OnSendMessage(const asio::error_code &code, std::shared_ptr<Tcp::ProtoMessage> message) final;
	 private:
        int mTimeout;
		HttpComponent * mHttpComponent;
        std::shared_ptr<asio::steady_timer> mTimer;
        std::shared_ptr<HttpAsyncRequest> mRequest;
		std::shared_ptr<HttpAsyncResponse> mResponse;
	};
}
#endif //GAMEKEEPER_HTTPREQUESTCLIENT_H