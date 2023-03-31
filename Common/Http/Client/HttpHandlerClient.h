//
// Created by zmhy0073 on 2022/1/19.
//

#ifndef GAMEKEEPER_HTTPHANDLERCLIENT_H
#define GAMEKEEPER_HTTPHANDLERCLIENT_H

#include"Http.h"
#include"Network/Tcp/TcpContext.h"
namespace Http
{
    class Request;
    class Response;
}
namespace Sentry
{
	class HttpListenComponent;
	class HttpHandlerClient final : public Tcp::TcpContext
	{
	 public:
		HttpHandlerClient(HttpListenComponent * httpComponent, std::shared_ptr<SocketProxy> socketProxy);
	 public:
		void StartReceive(int timeout = 15);
		void StartWriter(HttpStatus code);
		void StartWriter(const std::shared_ptr<Http::Response>& message);
        std::shared_ptr<Http::Request> Request() { return this->mHttpRequest;}
	 private:
		void ClosetClient();
		void OnTimeout(const Asio::Code& code);
        void OnReceiveMessage(const Asio::Code &code, std::istream & is, size_t) final;
        void OnReceiveLine(const Asio::Code &code, std::istream &readStream, size_t size) final;
        void OnSendMessage(const asio::error_code &code, std::shared_ptr<Tcp::ProtoMessage> message) final;
	 private:
        int mTimeout;
        std::string mMethod;
		std::shared_ptr<Asio::Timer> mTimer;
        HttpListenComponent * mHttpComponent;
        std::shared_ptr<Http::Request> mHttpRequest;		
	};
}
#endif //GAMEKEEPER_HTTPHANDLERCLIENT_H
