//
// Created by yy on 2024/7/5.
//

#ifndef APP_WATCH_H
#define APP_WATCH_H
#include "Http/Service/HttpService.h"
namespace acs
{
	class Watch : public HttpService
	{
	public:
		Watch();
		~Watch() final = default;
	private:
		bool OnInit() final;
	private:
		int List(const http::FromContent& request, json::w::Document& response);
		int StartProcess(const json::r::Document & request, json::w::Document & response);
		int CloseProcess(const http::FromContent & request, json::w::Document & response);
	private:
		class WatchComponent * mWatch;
	};
}

#endif //APP_WATCH_H
