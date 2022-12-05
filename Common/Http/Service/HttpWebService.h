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
        XCode Info(const Http::Request& request, Http::Response& response);
		XCode Ping(const Http::Request& request, Http::Response& response);
        XCode Hello(const Http::Request& request, Http::Response& response);
		XCode Hotfix(const Http::Request& request, Http::Response& response);
		XCode DownLoad(const Http::Request& request, Http::Response& response);
        XCode Sleep(const Json::Reader & request, Json::Document & response);
    };
}


#endif //APP_HTTPWEBSERVICE_H
