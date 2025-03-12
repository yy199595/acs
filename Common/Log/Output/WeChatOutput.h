//
// Created by leyi on 2023/12/25.
//

#ifndef APP_WECHATOUTPUT_H
#define APP_WECHATOUTPUT_H
#include"Log/Common/Logger.h"
#ifdef __ENABLE_OPEN_SSL__
#include<asio/ssl/context.hpp>
#endif
#include"Http/Client/HttpClient.h"
namespace custom
{
	class WeChatOutput : public IOutput
	{
	public:
		WeChatOutput(std::string  url, std::string  pem);
	private:
		bool Start(Asio::Context &io) final;
		void Push(Asio::Context &io, const std::string &name, const custom::LogInfo &logInfo) final;
	private:
		std::string mUrl;
#ifdef __ENABLE_OPEN_SSL__
		std::string mPem;
		asio::ssl::context mCtx;
#endif
		std::shared_ptr<http::Client> mClient;
		std::queue<std::unique_ptr<http::Request>> mMessages;
	};
}


#endif //APP_WECHATOUTPUT_H
