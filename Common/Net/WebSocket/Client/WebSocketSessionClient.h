//
// Created by 64658 on 2025/1/2.
//

#ifndef APP_WEBSOCKETSESSIONCLIENT_H
#define APP_WEBSOCKETSESSIONCLIENT_H
#include "Network/Tcp/Client.h"
#include "Entity/Component/IComponent.h"
#include "WebSocket/Common/WebSocketMessage.h"

namespace http
{
	class Request;
	class Response;
}


namespace ws
{
	typedef acs::IRpc<ws::Message, ws::Message> Component;
	class SessionClient : public tcp::Client
	{
	public:
		SessionClient(int id, Component * component, Asio::Context & main);
		~SessionClient() final;
	public:
		void StartWrite(ws::Message * message);
		void StartReceive(tcp::Socket * tcpSocket);
	private:
		void OnSendMessage() final;
		void OnReadError(const Asio::Code &code) final;
		void OnSendMessage(const Asio::Code &code) final;
		void OnReceiveLine(std::istream &readStream, size_t size) final;
		void OnReceiveMessage(std::istream &readStream, size_t size, const asio::error_code &code) final;
	private:
		void OnReadBody();
		bool DecodeByHttp();
		void Close(int code);
		void StartReceiveWebSocket();
	private:
		bool mIsHttp;
		int mSockId;
		Component * mComponent;
		Asio::Context & mMainContext;
		http::Request * mHttpRequest;
		std::unique_ptr<ws::Message> mMessage;
		std::queue<ws::Message *> mWaitSendMessage;
	};
}



#endif //APP_WEBSOCKETSESSIONCLIENT_H