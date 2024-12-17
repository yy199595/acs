//
// Created by leyi on 2023/12/25.
//

#ifndef APP_DINGDINGOUTPUT_H
#define APP_DINGDINGOUTPUT_H
#include"Log/Common/Logger.h"
#include<asio/ssl/context.hpp>
#include"Http/Client/RequestClient.h"
namespace custom
{
	class DingDingOutput : public IOutput
	{
	public:
		DingDingOutput(std::string  url, std::string  pem);
	private:
		bool Start(Asio::Context &io) final;
		void Push(Asio::Context &io, const std::string &name, const custom::LogInfo &logInfo) final;
	private:
		std::string mUrl;
#ifdef __ENABLE_OPEN_SSL__
		std::string mPem;
		asio::ssl::context mCtx;
#endif
		std::shared_ptr<http::RequestClient> mClient;
	};
}


#endif //APP_DINGDINGOUTPUT_H
