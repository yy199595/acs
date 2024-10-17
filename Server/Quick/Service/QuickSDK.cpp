//
// Created by yy on 2024/6/26.
//

#include "QuickSDK.h"
#include <regex>
#include "Util/Crypt/md5.h"
#include "Entity/Actor/App.h"
#include "XML/Document/XDocument.h"
#include "Http/Component/HttpComponent.h"

#define LOG_CHECK_BREAK(obj) { if(!obj) { break; }}

namespace acs
{
	QuickSDK::QuickSDK()
	{
		this->mHttp = nullptr;
	}

	bool QuickSDK::Awake()
	{
		std::unique_ptr<json::r::Value> jsonObject;
		LOG_CHECK_RET_FALSE(this->mApp->Config().Get("quick", jsonObject))
		LOG_CHECK_RET_FALSE(jsonObject->Get("md5_key", this->mConfig.md5_key))
		LOG_CHECK_RET_FALSE(jsonObject->Get("callback_key", this->mConfig.callback_key))
		return true;
	}

	bool QuickSDK::OnInit()
	{
		BIND_COMMON_HTTP_METHOD(QuickSDK::OnUserPay);
		BIND_COMMON_HTTP_METHOD(QuickSDK::OnUserLogin);
		LOG_CHECK_RET_FALSE(this->mHttp = this->GetComponent<HttpComponent>())
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
			http::FromContent fromData;
			if (!fromData.Decode(customData->Content()))
			{
				break;
			}
			std::string nt_data, sign, md5_sign;
			LOG_CHECK_BREAK(fromData.Get("sign", sign));
			LOG_CHECK_BREAK(fromData.Get("nt_data", nt_data));
			LOG_CHECK_BREAK(fromData.Get("md5Sign", md5_sign));
			std::string input = fmt::format("{}{}{}", nt_data, sign, this->mConfig.md5_key);
			if (help::md5::GetMd5(input) != md5_sign)
			{
				break;
			}
			std::string output;
			if (!this->Decode(nt_data, output))
			{
				break;
			}
			xml::XDocument xmlDocument;
			if (!xmlDocument.Decode(output))
			{
				break;
			}
			std::unique_ptr<xml::XElement> xmlElement;
			if (!xmlDocument.Get("message", xmlElement))
			{
				break;
			}
			quick::PayNotify payNotify;
			xmlElement->Get("is_test", payNotify.is_test);
			xmlElement->Get("extra_params", payNotify.extras_params);
			xmlElement->Get("original_amount", payNotify.original_amount);
			xmlElement->Get("original_currency", payNotify.original_currency);
			LOG_ERROR_CHECK_ARGS(xmlElement->Get("status", payNotify.status))
			LOG_ERROR_CHECK_ARGS(xmlElement->Get("amount", payNotify.amount))
			LOG_ERROR_CHECK_ARGS(xmlElement->Get("channel", payNotify.channel))
			LOG_ERROR_CHECK_ARGS(xmlElement->Get("order_no", payNotify.order_no))
			LOG_ERROR_CHECK_ARGS(xmlElement->Get("pay_time", payNotify.pay_time))
			LOG_ERROR_CHECK_ARGS(xmlElement->Get("order_no", payNotify.order_no))
			LOG_ERROR_CHECK_ARGS(xmlElement->Get("game_order", payNotify.game_order))
			LOG_ERROR_CHECK_ARGS(xmlElement->Get("channel_uid", payNotify.channel_uid))

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
		const std::string host("http://checkuser.quickapi.net/v2/checkUserInfo");
		const std::string url = fmt::format("{}?token={}&uid={}", host, token, uid);
		http::Response * response1 = this->mHttp->Get(url);
		if(response1 == nullptr || response1->Code() != HttpStatus::OK)
		{
			return XCode::Failure;
		}
		const http::TextContent * customData = response1->To<const http::TextContent>();
		if(customData == nullptr)
		{
			return XCode::Failure;
		}
		const std::string & content = customData->Content();
		size_t pos = content.find("\r\n");
		if(pos == std::string::npos)
		{
			return XCode::Failure;
		}
		std::string result = content.substr(0, pos);
		return XCode::Ok;
	}

	bool QuickSDK::Decode(const std::string& input, std::string& output)
	{
		if(input.empty())
		{
			return false;
		}
		std::smatch results;
		std::regex pattern("\\d+");
		std::vector<std::string> list;

		auto end = std::sregex_iterator();
		auto iter = std::sregex_iterator(input.begin(), input.end(), pattern);
		for(; iter != end; iter++)
		{
			list.emplace_back(iter->str());
		}
		if(list.empty()) {
			return false;
		}
		std::vector<char> data(list.size());
		std::vector<char> keys(this->mConfig.callback_key.begin(), this->mConfig.callback_key.end());

		for (size_t i = 0; i < data.size(); ++i) {
			int num = std::stoi(list[i]);
			data[i] = static_cast<char>(num - (0xff & static_cast<int>(keys[i % keys.size()])));
		}
		output.assign(data.begin(), data.end());
		return true;
	}
}