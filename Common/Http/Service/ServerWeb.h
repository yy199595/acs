//
// Created by zmhy0073 on 2022/8/29.
//

#ifndef APP_SERVERWEB_H
#define APP_SERVERWEB_H
#include"PhysicalHttpService.h"
namespace Sentry
{
    class ServerWeb : public PhysicalHttpService
    {
    public:
        ServerWeb() = default;
        ~ServerWeb() final = default;
    private:
		bool Awake();
        bool OnInit() final;
    private:
        int Info(Json::Writer & response);
		int Hotfix(Json::Writer& response);
		int Main(const Http::Request& request, Http::Response& response);
		int Ping(const Http::Request& request, Http::Response& response);
		int Hello(const Http::Request& request, Http::Response& response);
		int DownLoad(const Http::Request& request, Http::Response& response);
		int Sleep(const Json::Reader & request, Json::Writer& response);
	private:
		std::unordered_map<std::string, std::string> mHtmlFiles;
    };
}


#endif //APP_SERVERWEB_H
