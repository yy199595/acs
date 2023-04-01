//
// Created by zmhy0073 on 2022/8/29.
//

#ifndef APP_SERVERWEB_H
#define APP_SERVERWEB_H
#include"Message/s2s.pb.h"
#include"PhysicalHttpService.h"
namespace Sentry
{
    class ServerWeb : public PhysicalHttpService
    {
    public:
        ServerWeb() = default;
        ~ServerWeb() final = default;
    private:
        bool OnInit() final;
		int QueryServer(std::shared_ptr<s2s::server::list> & response);
    private:
		int Stop(Json::Writer & response);
        int Info(Json::Writer & response);
		int Hotfix(Json::Writer& response);
        int Login(const Http::Request& request, Http::DataResponse& response);
        int Register(const Http::Request& request, Http::DataResponse& response);
    };
}


#endif //APP_SERVERWEB_H
