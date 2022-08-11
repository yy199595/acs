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
	class HttpListenComponent;
	class HttpHandlerClient final : public Tcp::TcpContext
	{
	 public:
		HttpHandlerClient(HttpListenComponent * httpComponent, std::shared_ptr<SocketProxy> socketProxy);
	 public:
		void StartReceive();
        void StartWriter(HttpStatus code);
        std::shared_ptr<HttpHandlerRequest> Request() { return this->mHttpRequest;}
		std::shared_ptr<HttpHandlerResponse> Response() { return this->mHttpResponse;}
	 private:
		void OnComplete();
		void ClosetClient();
        void OnReceiveLine(const asio::error_code &code, std::istream &is, size_t) final;
        void OnReceiveMessage(const asio::error_code &code, std::istream & is, size_t) final;
        void OnSendMessage(const asio::error_code &code, std::shared_ptr<Tcp::ProtoMessage> message) final;
	 private:
        HttpListenComponent * mHttpComponent;
        std::shared_ptr<HttpHandlerRequest> mHttpRequest;
		std::shared_ptr<HttpHandlerResponse> mHttpResponse;
	};
}
#endif //GAMEKEEPER_HTTPHANDLERCLIENT_H
