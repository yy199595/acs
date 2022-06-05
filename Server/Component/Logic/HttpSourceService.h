//
// Created by yjz on 2022/6/5.
//

#ifndef _HTTPSOURCESERVICE_H_
#define _HTTPSOURCESERVICE_H_
#include"Component/HttpService/LocalHttpService.h"
namespace Sentry
{
	class HttpSourceService : public LocalHttpService
	{
	 public:
		HttpSourceService() = default;
		~HttpSourceService() = default;
	 private:
		bool OnStartService(HttpServiceRegister &serviceRegister) final;
	 private:
		XCode Files(const HttpHandlerRequest& request, HttpHandlerResponse& response);
		XCode Download(const HttpHandlerRequest& request, HttpHandlerResponse& response);
	 private:
		std::string mSourcePath;
	};
}

#endif //_HTTPSOURCESERVICE_H_
