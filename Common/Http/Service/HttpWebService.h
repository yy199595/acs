//
// Created by zmhy0073 on 2022/8/29.
//

#ifndef APP_HTTPWEBSERVICE_H
#define APP_HTTPWEBSERVICE_H
#include"LocalHttpService.h"
namespace Sentry
{
    class HttpWebService : public LocalHttpService
    {
    public:
        HttpWebService() = default;
        ~HttpWebService() = default;
    private:
        bool OnStartService(HttpServiceRegister &serviceRegister) final;
		XCode Ping(const HttpHandlerRequest& request, HttpHandlerResponse& response);
		XCode Sleep(const HttpHandlerRequest& request, HttpHandlerResponse& response);
        XCode Hello(const HttpHandlerRequest& request, HttpHandlerResponse& response);
        XCode DownLoad(const HttpHandlerRequest& request, HttpHandlerResponse& response);
    };
}


#endif //APP_HTTPWEBSERVICE_H
