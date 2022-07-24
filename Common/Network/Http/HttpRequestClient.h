//
// Created by yjz on 2022/1/19.
//

#ifndef GAMEKEEPER_HTTPREQUESTCLIENT_H
#define GAMEKEEPER_HTTPREQUESTCLIENT_H
#include<Async/TaskSource.h>
#include"HttpAsyncRequest.h"
#include"Network/TcpContext.h"
namespace Sentry
{
	class HttpComponent;
	class HttpRequestClient : public Tcp::TcpContext
	{
	 public:
		HttpRequestClient(std::shared_ptr<SocketProxy> socketProxy, HttpComponent * component);
	 public:
		void Request(std::shared_ptr<HttpAsyncRequest> request);
		void Request(std::shared_ptr<HttpAsyncRequest> request, std::fstream * fs);
	 private:
        void ConnectHost();
        void OnComplete(const asio::error_code & code);
        void OnConnect(const asio::error_code &error, int count) final;
        void OnReceiveLine(const asio::error_code &code, size_t) final;
        void OnReceiveMessage(const asio::error_code &code, size_t) final;
		void OnSendMessage(const asio::error_code &code, std::shared_ptr<Tcp::ProtoMessage> message) final;
	 private:
		HttpComponent * mHttpComponent;
		std::shared_ptr<HttpAsyncRequest> mRequest;
		std::shared_ptr<HttpAsyncResponse> mResponse;
	};
}
#endif //GAMEKEEPER_HTTPREQUESTCLIENT_H
