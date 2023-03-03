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
        int Info(Json::Writer & response);
		int Hotfix(Json::Writer& response);
		int Ping(const Http::Request& request, Http::Response& response);
		int Hello(const Http::Request& request, Http::Response& response);
		int DownLoad(const Http::Request& request, Http::Response& response);
		int Sleep(const Json::Reader & request, Json::Writer& response);
    };
}


#endif //APP_HTTPBACKGROUND_H
