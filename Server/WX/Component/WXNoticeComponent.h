//
// Created by yy on 2024/5/18.
//

#ifndef APP_WXNOTICECOMPONENT_H
#define APP_WXNOTICECOMPONENT_H
#include "unordered_map"
#include "Entity/Component/Component.h"

namespace wx
{
	struct WhcbqhnConfig
	{
		std::string app_id;
		std::string app_secret;
	};
	struct Token
	{
		std::string token;
		long long exp_time = 0;
	};

	struct NoticeData
	{
		std::string path;
		std::string templateId;
		std::unordered_map<std::string, std::string> data;
	};
}

namespace acs
{
	//微信公众号通知
	class WXNoticeComponent final : public Component, public IComplete
	{
	public:
		WXNoticeComponent();
		~WXNoticeComponent() final = default;
	private:
		bool Awake() final;
		bool LateAwake() final;
		void Complete() final;
	public:
		void OnWxSubscribe(const std::string & openId);
		void OnWxUnsubscribe(const std::string & openId);
		static std::string Truncate(const std::string & thing, int count = 20);
		bool Send(const std::string & openId, const wx::NoticeData & noticeData);
	private:
		bool GetAccessToken();
	private:
		wx::Token mToken;
		std::string mAppId;
		wx::WhcbqhnConfig mConfig;
		class HttpComponent * mHttp;
		std::unordered_map<int, std::string> mOpenIdMap;
	};
}


#endif //APP_WXNOTICECOMPONENT_H
