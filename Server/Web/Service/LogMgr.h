//
// Created by leyi on 2023/11/6.
//

#ifndef APP_LOGMGR_H
#define APP_LOGMGR_H
#include"Http/Service/HttpService.h"

namespace acs
{
	class LogMgr final : public HttpService
	{
	public:
		LogMgr() = default;
	private:
		bool Awake() final;
		bool OnInit() final;
	private:
		int Html(const http::FromContent& request, http::Response& response);
		int Look(const http::FromContent& request, http::Response& response);
		int List(const http::FromContent & request,  http::Response & response);
		int Delete(const http::FromContent & request, json::w::Value & response);
		int SetLevel(const http::FromContent & request, json::w::Value & response);
	private:
		std::string mLogDir;
	};
}




#endif //APP_LOGMGR_H
