//
// Created by yy on 2024/6/30.
//

#ifdef __ENABLE_OPEN_SSL__

#include "WXComplaintComponent.h"
#include "XCode/XCode.h"
#include "Util/Tools/TimeHelper.h"
#include "Http/Component/NotifyComponent.h"
#include "Http/Common/Content.h"
#include "Config/Base/LangConfig.h"
#include "WX/Component/WeChatComponent.h"

namespace acs
{
	WXComplaintComponent::WXComplaintComponent()
	{
		this->mWeChat = nullptr;
	}

	bool WXComplaintComponent::Awake()
	{
		LOG_CHECK_RET_FALSE(LangConfig::Inst()->Get("pay_time", this->mText.pay_time))
		LOG_CHECK_RET_FALSE(LangConfig::Inst()->Get("user_city", this->mText.user_city))
		LOG_CHECK_RET_FALSE(LangConfig::Inst()->Get("user_nick", this->mText.user_nick))
		LOG_CHECK_RET_FALSE(LangConfig::Inst()->Get("order_desc", this->mText.order_desc))
		LOG_CHECK_RET_FALSE(LangConfig::Inst()->Get("order_remark", this->mText.order_remark))
		LOG_CHECK_RET_FALSE(LangConfig::Inst()->Get("create_time", this->mText.create_time))
		LOG_CHECK_RET_FALSE(LangConfig::Inst()->Get("expire_time", this->mText.expire_time))
		LOG_CHECK_RET_FALSE(LangConfig::Inst()->Get("order_amount", this->mText.order_amount))
		LOG_CHECK_RET_FALSE(LangConfig::Inst()->Get("phone_number", this->mText.phone_number))
		LOG_CHECK_RET_FALSE(LangConfig::Inst()->Get("complaint_reason", this->mText.complaint_reason))
		LOG_CHECK_RET_FALSE(LangConfig::Inst()->Get("order_complaint_notify", this->mText.order_complaint_notify))
		return true;
	}

	bool WXComplaintComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->mWeChat = this->GetComponent<WeChatComponent>())
		return true;
	}

	int WXComplaintComponent::OnComplaint(const json::r::Document& document)
	{
		wx::OrderComplaint orderComplaint;
		LOG_ERROR_CHECK_ARGS(document.Get("status", orderComplaint.status))
		LOG_ERROR_CHECK_ARGS(document.Get("open_id", orderComplaint.open_id))
		LOG_ERROR_CHECK_ARGS(document.Get("pay_time", orderComplaint.pay_time))
		LOG_ERROR_CHECK_ARGS(document.Get("total_cost", orderComplaint.total_cost))
		LOG_ERROR_CHECK_ARGS(document.Get("create_time", orderComplaint.create_time))
		LOG_ERROR_CHECK_ARGS(document.Get("expire_time", orderComplaint.expire_time))
		LOG_ERROR_CHECK_ARGS(document.Get("out_trade_no", orderComplaint.out_trade_no))
		LOG_ERROR_CHECK_ARGS(document.Get("phone_number", orderComplaint.phone_number))
		LOG_ERROR_CHECK_ARGS(document.Get("product_name", orderComplaint.product_name))
		LOG_ERROR_CHECK_ARGS(document.Get("complaint_order_id", orderComplaint.complaint_order_id))

		if(orderComplaint.status != "201") //待处理
		{
			return XCode::Ok;
		}

		std::unique_ptr<json::r::Value> jsonValue;
		if(document.Get("history", jsonValue))
		{
			jsonValue->Get("content", orderComplaint.reason);
		}
		return XCode::Ok;
	}

	int WXComplaintComponent::FindInfo(const std::string& id, json::w::Document& response)
	{
		http::FromContent fromData;
		fromData.Add("complaintOrderId", id);
		std::string host("https://api.weixin.qq.com/wxaapi/minishop/complaintOrderDetail");
		const http::JsonContent * jsonData = this->mWeChat->Get(host, fromData);
		if(jsonData == nullptr)
		{
			return XCode::RequestWxApiError;
		}
		response.AddObject("data", jsonData->JsonStr());
		return XCode::Ok;
	}

}

#endif