//
// Created by yjz on 2022/1/19.
//

#ifndef APP_REQUESTCLIENT_H
#define APP_REQUESTCLIENT_H
#include"Http.h"
#include"Network/Tcp/Client.h"
#include"Http/Common/HttpRequest.h"
#include"Http/Common/HttpResponse.h"
#include"Entity/Component/IComponent.h"

namespace http
{
	class RequestClient : public tcp::Client
#ifdef __SHARE_PTR_COUNTER__
			, public memory::Object<RequestClient>
#endif
	{
	 public:
		typedef acs::IRpc<Request, Response> Component;
		explicit RequestClient(Component * component, Asio::Context & io);
	 public:
		bool SyncSend(std::unique_ptr<http::Request> request);
		bool SyncSend(std::unique_ptr<http::Request> request, http::Response & response);
		void Do(std::unique_ptr<http::Request> request, std::unique_ptr<http::Response> response, int taskId);
	 private:
        void OnComplete(HttpStatus code);
		void OnTimeout(tcp::TimeoutFlag flag) final;
		void OnConnect(bool result, int count) final;
		void OnReadError(const Asio::Code &code) final;
        void OnReceiveLine(std::istream &is, size_t) final;
        void OnReceiveMessage(std::istream & is, size_t, const Asio::Code &) final;
	private:
		void OnSendMessage(size_t size) final;
		void OnSendMessage(const asio::error_code &code) final;
	 private:
		Component * mComponent;
		Asio::Context & mMainContext;
        std::unique_ptr<http::Request> mRequest;
		std::unique_ptr<http::Response> mResponse;
    };
}
#endif //APP_REQUESTCLIENT_H
