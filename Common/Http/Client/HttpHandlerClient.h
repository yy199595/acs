//
// Created by zmhy0073 on 2022/1/19.
//

#ifndef GAMEKEEPER_HTTPHANDLERCLIENT_H
#define GAMEKEEPER_HTTPHANDLERCLIENT_H
#include"Http/HttpRequest.h"
#include"Http/HttpResponse.h"
#include"Source/TaskSource.h"
#include"Tcp/SocketProxy.h"
#include"Json/JsonWriter.h"
#include"Tcp/TcpContext.h"
namespace Sentry
{
	class HttpListenComponent;
	class HttpHandlerClient final : public Tcp::TcpContext
	{
	 public:
		HttpHandlerClient(HttpListenComponent * httpComponent, std::shared_ptr<SocketProxy> socketProxy);
	 public:
		void StartWriter();
		void StartReceive();
        std::shared_ptr<Http::Request> Request() { return this->mHttpRequest;}
		std::shared_ptr<Http::Response> Response() { return this->mHttpResponse;}
	 private:
		void ClosetClient();
        void OnReceiveMessage(const asio::error_code &code, std::istream & is, size_t) final;
        void OnReceiveLine(const Asio::Code &code, std::istream &readStream, size_t size) final;
        void OnSendMessage(const asio::error_code &code, std::shared_ptr<Tcp::ProtoMessage> message) final;
	 private:
        std::string mMethod;
        Http::DecodeState mDecodeState;
        HttpListenComponent * mHttpComponent;
        std::shared_ptr<Http::Request> mHttpRequest;
		std::shared_ptr<Http::Response> mHttpResponse;
	};
}
#endif //GAMEKEEPER_HTTPHANDLERCLIENT_H
