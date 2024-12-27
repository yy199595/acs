//
// Created by yy on 2024/6/26.
//

#include "QuickSDK.h"
#include "Entity/Actor/App.h"
#include "Quick/Component/QuickComponent.h"

namespace acs
{
	QuickSDK::QuickSDK()
	{
		this->mQuick = nullptr;
	}

	bool QuickSDK::Awake()
	{
		this->mApp->AddComponent<QuickComponent>();
		return true;
	}

	bool QuickSDK::OnInit()
	{
		BIND_COMMON_HTTP_METHOD(QuickSDK::OnUserPay);
		BIND_COMMON_HTTP_METHOD(QuickSDK::OnUserLogin);
		LOG_CHECK_RET_FALSE(this->mQuick = this->GetComponent<QuickComponent>())
		return true;
	}

	int QuickSDK::OnUserPay(const http::Request& request, http::Response& response)
	{
		int code = XCode::Failure;
		std::string result = "FAILED";
		do
		{
			const http::TextContent* customData = request.GetBody()->To<const http::TextContent>();
			if (customData == nullptr || customData->Content().empty())
			{
				break;
			}
			quick::PayNotify payNotify;
			if(!this->mQuick->Decode(customData->Content(), payNotify))
			{
				break;
			}

			code = XCode::Ok;
			result = "SUCCESS";
		} while (false);
		response.SetContent(http::Header::TEXT, result);
		return code;
	}

	int QuickSDK::OnUserLogin(const json::r::Document& request, json::w::Document& response)
	{
		std::string token, uid;
		LOG_ERROR_CHECK_ARGS(request.Get("uid", uid))
		LOG_ERROR_CHECK_ARGS(request.Get("token", token))
		if(!this->mQuick->Login(uid, token))
		{
			return XCode::Failure;
		}
		return XCode::Ok;
	}
}