//
// Created by yjz on 2023/3/23.
//

#ifndef APP_COMMON_HTTP_SERVICE_PHYSICALHTTPSERVICE_H
#define APP_COMMON_HTTP_SERVICE_PHYSICALHTTPSERVICE_H
#include"HttpService.h"
namespace Sentry
{
	class PhysicalHttpService : public HttpService
	{
	public:
		explicit PhysicalHttpService();
	private:
		bool IsStartService() final { return true;}
		int Invoke(const std::string & name,
			const std::shared_ptr<Http::Request>&, std::shared_ptr<Http::Response> &) final;
	private:
		unsigned int mSumCount;
		unsigned int mWaitCount;
	};
}

#endif //APP_COMMON_HTTP_SERVICE_PHYSICALHTTPSERVICE_H
