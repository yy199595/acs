//
// Created by leyi on 2023/11/6.
//

#ifndef APP_LOGMGR_H
#define APP_LOGMGR_H
#include"Http/Service/HttpService.h"

namespace joke
{
	class LogMgr : public HttpService
	{
	public:
		LogMgr() = default;
	private:
		bool Awake() final;
		bool OnInit() final;
	private:
		int Look(const http::FromData & request,  http::Response & response);
		int List(const http::FromData & request,  http::Response & response);
		int Delete(const http::FromData & request, json::w::Value & response);
		int SetLevel(const http::FromData & request, json::w::Value & response);
	private:
		std::string mLogDir;
	};
}




#endif //APP_LOGMGR_H
