//
// Created by zmhy0073 on 2022/1/19.
//

#ifndef APP_SESSIONCLIENT_H
#define APP_SESSIONCLIENT_H

#include"Http.h"
#include"Network/Tcp/Client.h"
#include"Http/Common/HttpRequest.h"
#include"Http/Common/HttpResponse.h"
#include"Entity/Component/IComponent.h"

namespace http
{
	class SessionClient final : public tcp::Client
#ifdef __SHARE_PTR_COUNTER__
			, public memory::Object<SessionClient>
#endif
	{
	 public:
		typedef acs::IRpc<Request, Response> Component;
		explicit SessionClient(Component * component, Asio::Context & io);
		~SessionClient();
	 public:
		bool StartWriter();
		void StartClose(int code);
		bool StartWriter(HttpStatus status);
		bool StartWriter(HttpStatus status, std::unique_ptr<Content> data);
		void StartReceive(int id, tcp::Socket * socket, int timeout = 10);
	public:
		void StartReceiveBody();
		void StartReceiveBody(std::unique_ptr<http::Content> content);
	private:
        void OnReadPause();
		void ClosetClient(int code);
		void OnComplete(HttpStatus status);
		void OnTimeout(tcp::TimeoutFlag flag) final;
		void OnReadError(const Asio::Code &code) final;
		void OnReceiveLine(std::istream &is, size_t) final;
        void OnReceiveMessage(std::istream & is, size_t, const Asio::Code &) final;
	private:
		void Clear();
		void OnSendMessage(size_t size) final;
        void OnSendMessage(const asio::error_code &code) final;
	public:
		std::string mPath;
	 private:
		Component * mComponent;
		http::Request mRequest;
		http::Response mResponse;
		Asio::Context & mMainContext;
	};
}
#endif //APP_SESSIONCLIENT_H
