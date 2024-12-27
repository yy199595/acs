//
// Created by yy on 2024/6/26.
//

#ifndef APP_QUICKSDK_H
#define APP_QUICKSDK_H
#include "Http/Service/HttpService.h"


namespace acs
{
	class QuickSDK : public HttpService
	{
	public:
		QuickSDK();
		~QuickSDK() final = default;
	private:
		bool Awake() final;
		bool OnInit() final;
	private:
		int OnUserPay(const http::Request & request, http::Response & response);
		int OnUserLogin(const json::r::Document & request, json::w::Document & response);
	private:
		class QuickComponent * mQuick;
	};
}


#endif //APP_QUICKSDK_H
