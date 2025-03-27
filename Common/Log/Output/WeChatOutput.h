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
		explicit WeChatOutput(std::string url);
	private:
		bool Start(Asio::Context &io) final;
		void Push(Asio::Context &io, const std::string &name, const custom::LogInfo &logInfo) final;
	private:
		std::string mUrl;
#ifdef __ENABLE_OPEN_SSL__
		asio::ssl::context mCtx;
#endif
		std::shared_ptr<http::Client> mClient;
	};
}


#endif //APP_WECHATOUTPUT_H
