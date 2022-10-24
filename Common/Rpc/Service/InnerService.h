//
// Created by zmhy0073 on 2022/10/8.
//

#ifndef APP_INNERSERVICE_H
#define APP_INNERSERVICE_H
#include"LocalRpcService.h"
namespace Sentry
{
    class InnerService : public LocalRpcService
    {
    public:
        InnerService() = default;
        ~InnerService() = default;
    private:
        XCode Ping();
        XCode Stop();
        XCode Hotfix();
        XCode LoadConfig();
        XCode Join(const s2s::cluster::join & request); //新服务器加入
        XCode Exit(const s2s::cluster::exit & response); //新服务器加入
		void OnInvoke(const std::string & message);
		void OnInvoke2(const std::string & message);
	 private:
        bool Awake();
        bool OnStart() final;
        bool OnClose() final { return false; }
    private:
        class LocationComponent * mLocationComponent;
    };
}


#endif //APP_INNERSERVICE_H
