//
// Created by 64658 on 2025/1/2.
//

#ifndef APP_WS_SESSION_H
#define APP_WS_SESSION_H
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
	class Session final : public tcp::Client
	{
	public:
		Session(int id, Component * component, Asio::Context & main);
		~Session() final;
	public:
		void Stop();
		void Send(rpc::Message * message);
		void Send(ws::Message * message);
		void StartReceive(tcp::Socket * tcpSocket);
		inline void BindPlayerID(long long id) { this->mPlayerId = id; }
	private:
		void OnSendMessage(size_t size) final;
		void OnReadError(const Asio::Code &code) final;
		void OnSendMessage(const Asio::Code &code) final;
		void OnReceiveLine(std::istream &readStream, size_t size) final;
		void OnReceiveMessage(std::istream &readStream, size_t size, const asio::error_code &code) final;
	private:
		void OnPing();
		bool OnMessage();
		void AddToSendQueue(std::unique_ptr<ws::Message> message);
	private:
		void OnReadBody();
		bool DecodeByHttp();
		void Close(int code);
		void StartReceiveWebSocket();
	private:
		bool mIsHttp;
		int mSockId;
		long long mPlayerId;
		Component * mComponent;
		std::stringstream mStream;
		Asio::Context & mMainContext;
		http::Request * mHttpRequest;
		std::unique_ptr<ws::Message> mMessage;
		std::queue<std::unique_ptr<ws::Message>> mWaitSendMessage;
	};
}



#endif //APP_WS_SESSION_H
