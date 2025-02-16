//
// Created by yy on 2024/5/18.
//
#ifdef __ENABLE_OPEN_SSL__
#include "WeChat.h"
#include "Entity/Actor/App.h"
#include "WX/Component/WeChatComponent.h"
#include "WX/Component/WXNoticeComponent.h"
#include "WX/Component/WXComplaintComponent.h"

namespace acs
{
	WeChat::WeChat()
	{
		this->mWeChat = nullptr;
		this->mNotice = nullptr;
		this->mComplaint = nullptr;
	}

	bool WeChat::OnInit()
	{
		BIND_COMMON_HTTP_METHOD(WeChat::Event);
		BIND_COMMON_HTTP_METHOD(WeChat::PhoneNum);
		LOG_CHECK_RET_FALSE(this->mWeChat = this->GetComponent<WeChatComponent>())
		LOG_CHECK_RET_FALSE(this->mNotice = this->GetComponent<WXNoticeComponent>())
		LOG_CHECK_RET_FALSE(this->mComplaint = this->GetComponent<WXComplaintComponent>())

		return true;
	}

	int WeChat::Event(const http::Request& request, http::Response& response)
	{
		std::string openId, text;
		const http::Content* body = request.GetBody();
		const http::FromContent& query = request.GetUrl().GetQuery();
		if (query.Get("echostr", text))
		{
			std::string nonce;
			std::string signature;
			std::string timestamp;
			LOG_ERROR_CHECK_ARGS(query.Get("nonce", nonce));
			LOG_ERROR_CHECK_ARGS(query.Get("signature", signature));
			LOG_ERROR_CHECK_ARGS(query.Get("timestamp", timestamp));
			const std::string token = this->mApp->Config().GetSecretKey();
			std::vector<std::string> array = { token, timestamp, nonce };
			std::sort(array.begin(), array.end());
		}
		else if (this->mNotice != nullptr && body != nullptr && query.Get("openid", openId))
		{
			//CONSOLE_LOG_ERROR("body = {}", request.GetBody()->ToStr())
			const http::TextContent* customData = body->To<const http::TextContent>();
			if (customData != nullptr)
			{
				const std::string subscribe("[subscribe]");
				const std::string unsubscribe("[unsubscribe]");
				const std::string& message = customData->Content();
				if (message.find(subscribe) != std::string::npos)
				{
					this->mNotice->OnWxSubscribe(openId);
				}
				else if (message.find(unsubscribe) != std::string::npos)
				{
					this->mNotice->OnWxUnsubscribe(openId);
				}
			}
		}
		else if (body != nullptr && body->GetContentType() == http::ContentType::JSON)
		{
			std::string Event;
			const http::JsonContent* jsonData = body->To<const http::JsonContent>();
			if (jsonData == nullptr || (!jsonData->Get("EventSystem", Event)))
			{
				return XCode::CallArgsError;
			}
			if (Event != "complaint_callback")
			{
				return XCode::Ok;
			}
			return this->mComplaint->OnComplaint(*jsonData);
		}
		response.Text(text.c_str(), text.size());
		return XCode::Ok;
	}

	int WeChat::PhoneNum(const http::FromContent& request, json::w::Document& response)
	{
		int count = 0;
		std::string code;
		LOG_ERROR_CHECK_ARGS(request.Get("code", code));

		std::unique_ptr<wx::PhoneInfo> phoneInfo = this->mWeChat->GetPhoneNumByCode(code);
		if (phoneInfo == nullptr)
		{
			this->mWeChat->ClearAccessToken();
			phoneInfo = this->mWeChat->GetPhoneNumByCode(code);
		}
		if (phoneInfo == nullptr || phoneInfo->phoneNumber.empty())
		{
			return XCode::RequestWxApiError;
		}
		std::unique_ptr<json::w::Value> jsonObject = response.AddObject("data");
		{
			jsonObject->Add("phoneNumber", phoneInfo->phoneNumber);
			jsonObject->Add("countryCode", phoneInfo->countryCode);
			jsonObject->Add("purePhoneNumber", phoneInfo->purePhoneNumber);
		}
		return XCode::Ok;
	}
}
#endif