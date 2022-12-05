//
// Created by zmhy0073 on 2022/8/29.
//

#ifndef APP_HTTPBACKGROUND_H
#define APP_HTTPBACKGROUND_H
#include"LocalHttpService.h"
namespace Sentry
{
    class HttpBackGround : public LocalHttpService
    {
    public:
        HttpBackGround() = default;
        ~HttpBackGround() = default;
    private:     
        bool OnStartService(HttpServiceRegister &serviceRegister) final;
    private:
        XCode Info(Json::Document & response);
        XCode Hotfix(Json::Document & response);
        XCode Ping(const Http::Request& request, Http::Response& response);
        XCode Hello(const Http::Request& request, Http::Response& response);
		XCode DownLoad(const Http::Request& request, Http::Response& response);
        XCode Sleep(const Json::Reader & request, Json::Document & response);
    };
}


#endif //APP_HTTPBACKGROUND_H
