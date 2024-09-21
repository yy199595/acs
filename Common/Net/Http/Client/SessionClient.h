//
// Created by zmhy0073 on 2022/1/19.
//

#ifndef APP_SESSIONCLIENT_H
#define APP_SESSIONCLIENT_H

#include"Http.h"
#include"Network/Tcp/TcpClient.h"
#include"Http/Common/HttpRequest.h"
#include"Http/Common/HttpResponse.h"
#include"Entity/Component/IComponent.h"

namespace http
{
	class SessionClient final : public tcp::TcpClient
	{
	 public:
		typedef acs::IRpc<Request, Response> Component;
		explicit SessionClient(Component * component);
	 public:
		bool StartWriter();
		void StartReceiveBody();
		void StartClose(int code);
		bool StartWriter(HttpStatus status);
		void StartReceive(int id, tcp::Socket * socket, int timeout = 10);
	private:
        void OnReadPause();
		void ClosetClient(int code);
		void OnComplete(HttpStatus status);
		void OnTimeout(tcp::TimeoutFlag flag) final;
		void OnReadError(const Asio::Code &code) final;
		void OnReceiveLine(std::istream &is, size_t) final;
        void OnReceiveMessage(std::istream & is, size_t) final;
	public:

		const http::Request & GetRequest() const { return this->mRequest; }
		const http::Response & GetResponse() const { return this->mResponse; }
	private:
		void Clear();
		void OnSendMessage() final;
        void OnSendMessage(const asio::error_code &code) final;
	public:
		std::string mPath;
	 private:
		Component * mComponent;
		http::Request mRequest;
		http::Response mResponse;
	};
}
#endif //APP_SESSIONCLIENT_H
