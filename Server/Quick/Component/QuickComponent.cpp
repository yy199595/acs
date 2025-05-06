//
// Created by 64658 on 2024/12/27.
//

#include <regex>
#include "QuickComponent.h"
#include "Entity/Actor/App.h"

#include "Util/Tools/Math.h"
#include "Util/Crypt/MD5Helper.h"
#include "Http/Component/HttpComponent.h"
#include "Http/Common/Content.h"
#include "Http/Common/HttpResponse.h"
namespace acs
{
	QuickComponent::QuickComponent()
	{
		this->mHttp = nullptr;
		REGISTER_JSON_CLASS_MUST_FIELD(quick::Config, md5_key);
		REGISTER_JSON_CLASS_MUST_FIELD(quick::Config, callback_key);
	}

	bool QuickComponent::Awake()
	{
		LOG_CHECK_RET_FALSE(ServerConfig::Inst()->Get("quick", this->mConfig))
		return true;
	}

	bool QuickComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->mHttp = this->GetComponent<HttpComponent>())
		return true;
	}

	bool QuickComponent::Login(const std::string& uid, const std::string& token)
	{
		const std::string host("http://checkuser.quickapi.net/v2/checkUserInfo");
		const std::string url = fmt::format("{}?token={}&uid={}", host, token, uid);
		std::unique_ptr<http::Response> response1 = this->mHttp->Get(url);
		if(response1 == nullptr || response1->Code() != HttpStatus::OK)
		{
			return false;
		}
		const http::TextContent * customData = response1->To<const http::TextContent>();
		if(customData == nullptr)
		{
			return false;
		}
		const std::string & content = customData->Content();
		size_t pos = content.find("\r\n");
		if(pos == std::string::npos)
		{
			return false;
		}
		const std::string ok("1");
		return content.substr(0, pos) == ok;
	}

	bool QuickComponent::Decode(const std::string & message, quick::PayNotify & payNotify)
	{
		http::FromContent fromData;
		if (!fromData.Decode(message))
		{
			return false;
		}
		std::string nt_data, sign, md5_sign;
		LOG_CHECK_RET_FALSE(fromData.Get("sign", sign));
		LOG_CHECK_RET_FALSE(fromData.Get("nt_data", nt_data));
		LOG_CHECK_RET_FALSE(fromData.Get("md5Sign", md5_sign));
		std::string input = fmt::format("{}{}{}", nt_data, sign, this->mConfig.md5_key);
		if (help::md5::GetMd5(input) != md5_sign)
		{
			return false;
		}
		std::string output;
		if (!this->Decode(nt_data, output))
		{
			return false;
		}
		xml::XDocument xmlDocument;
		if (!xmlDocument.Decode(output))
		{
			return false;
		}
		std::unique_ptr<xml::XElement> xmlElement;
		if (!xmlDocument.Get("message", xmlElement))
		{
			return false;
		}
		xmlElement->Get("is_test", payNotify.is_test);
		xmlElement->Get("extra_params", payNotify.extras_params);
		xmlElement->Get("original_amount", payNotify.original_amount);
		xmlElement->Get("original_currency", payNotify.original_currency);
		LOG_CHECK_RET_FALSE(xmlElement->Get("status", payNotify.status))
		LOG_CHECK_RET_FALSE(xmlElement->Get("amount", payNotify.amount))
		LOG_CHECK_RET_FALSE(xmlElement->Get("channel", payNotify.channel))
		LOG_CHECK_RET_FALSE(xmlElement->Get("order_no", payNotify.order_no))
		LOG_CHECK_RET_FALSE(xmlElement->Get("pay_time", payNotify.pay_time))
		LOG_CHECK_RET_FALSE(xmlElement->Get("order_no", payNotify.order_no))
		LOG_CHECK_RET_FALSE(xmlElement->Get("game_order", payNotify.game_order))
		LOG_CHECK_RET_FALSE(xmlElement->Get("channel_uid", payNotify.channel_uid))
		return true;
	}

	bool QuickComponent::Decode(const std::string& input, std::string& output)
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