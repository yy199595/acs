//
// Created by zmhy0073 on 2022/8/29.
//

#ifndef APP_SERVERWEB_H
#define APP_SERVERWEB_H
#include"Http/Service/HttpService.h"
namespace Tendo
{
    class ServerWeb : public HttpService
    {
    public:
        ServerWeb() = default;
        ~ServerWeb() final = default;
    private:
        bool OnInit() final;
    private:
		int Stop(Json::Writer & response);
        int Info(Json::Writer & response);
		int Hotfix(Json::Writer& response);
        int Login(const Http::Request& request, Http::DataResponse& response);
        int Register(const Http::Request& request, Http::DataResponse& response);
    };
}


#endif //APP_SERVERWEB_H
