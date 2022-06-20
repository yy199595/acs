//
// Created by zmhy0073 on 2022/1/19.
//

#ifndef GAMEKEEPER_HTTPHANDLERCLIENT_H
#define GAMEKEEPER_HTTPHANDLERCLIENT_H
#include"HttpAsyncRequest.h"
#include"Async/TaskSource.h"
#include"Network/SocketProxy.h"
#include"Json/JsonWriter.h"
#include"Network/TcpContext.h"
namespace Sentry
{
	class HttpComponent;
	class HttpHandlerClient final : public Tcp::TcpContext
	{
	 public:
		HttpHandlerClient(HttpComponent * httpComponent, std::shared_ptr<SocketProxy> socketProxy);
	 public:
		void StartWriter();
		void StartReceive();
		std::shared_ptr<HttpHandlerRequest> Request() { return this->mHttpRequest;}
		std::shared_ptr<HttpHandlerResponse> Response() { return this->mHttpResponse;}
	 private:
		void OnComplete();
		void ClosetClient();
        void OnReceiveMessage(const asio::error_code &code, asio::streambuf &buffer) final;
		void OnSendMessage(const asio::error_code &code, std::shared_ptr<Tcp::ProtoMessage> message) final;
	 private:
		asio::streambuf mStreamBuffer;
		HttpComponent * mHttpComponent;
		std::shared_ptr<HttpHandlerRequest> mHttpRequest;
		std::shared_ptr<HttpHandlerResponse> mHttpResponse;
	};
}
#endif //GAMEKEEPER_HTTPHANDLERCLIENT_H
