//
// Created by yy on 2024/5/18.
//

#ifndef APP_WECHAT_H
#define APP_WECHAT_H
#include "Http/Service/HttpService.h"

namespace joke
{
	class WeChat : public HttpService
	{
	public:
		WeChat();
		~WeChat() = default;
	private:
		bool OnInit() final;
	private:
		int Event(const http::Request & request, http::Response &);
		int PhoneNum(const http::FromData & request, json::w::Document & response);
	private:
		class WeChatComponent * mWeChat;
		class WXNoticeComponent * mNotice;
	};
}


#endif //APP_WECHAT_H
