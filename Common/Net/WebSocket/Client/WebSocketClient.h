//
// Created by 64658 on 2025/1/2.
//

#ifndef APP_WEBSOCKETCLIENT_H
#define APP_WEBSOCKETCLIENT_H
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
	typedef acs::IRpc<rpc::Message, rpc::Message> Component;
	class RequestClient final : public tcp::Client
	{
	public:
		RequestClient(int id, Component * component, Asio::Context & main);
	public:
		void Close();
		void Send(ws::Message * message);
		void Send(rpc::Message * message);
	private:
		void OnUpdate() final;
		void OnSendMessage(size_t size) final;
		void OnConnect(bool result, int count) final;
		void OnReadError(const Asio::Code &code) final;
		void OnSendMessage(const Asio::Code &code) final;
		void OnReceiveLine(std::istream &readStream, size_t size) final;
		void OnReceiveMessage(std::istream &readStream, size_t size, const asio::error_code &code) final;
	private:
		void Close(int code);
		void SendFirstMessage();
		void AddToSendQueue(std::unique_ptr<ws::Message> message);
	private:
		int mSockId;
		Component * mComponent;
		std::stringstream mStream;
		Asio::Context & mMainContext;
		http::Request * mHttpRequest;
		http::Response * mHttpResponse;
		std::unique_ptr<ws::Message> mMessage;
		std::queue<std::unique_ptr<ws::Message>> mWaitSendMessage;
	};
}



#endif //APP_WEBSOCKETCLIENT_H
