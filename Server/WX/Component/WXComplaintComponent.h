//
// Created by yy on 2024/6/30.
//

#ifdef __ENABLE_OPEN_SSL__

#ifndef APP_WXCOMPLAINTCOMPONENT_H
#define APP_WXCOMPLAINTCOMPONENT_H

#include "Entity/Component/Component.h"

namespace wx
{
	struct OrderComplaint
	{
		std::string status;
		std::string pay_time;
		std::string total_cost;
		std::string create_time;
		std::string expire_time;
		std::string open_id;
		std::string phone_number;
		std::string product_name;
		std::string out_trade_no;
		std::string complaint_order_id;

		std::string reason;
	};

	struct UserComplaionInfo
	{
		int user_id = 0; //投诉人
		int target_id = 0; // 发起人
		int amount = 0; //订单金额
		int status = 0; //状态
		std::string id; //投诉单号
		std::string desc; //订单描述
		std::string order_id;
		std::string phone_num;
		std::string reason;
		long long create_time = 0; //创建时间
	};
}

namespace acs
{

	//微信投诉组件
	class WXComplaintComponent final : public Component
	{
		struct LangText
		{
			std::string user_nick;
			std::string order_amount;
			std::string phone_number;
			std::string user_city;
			std::string pay_time;
			std::string create_time;
			std::string expire_time;
			std::string complaint_reason;
			std::string order_desc;
			std::string order_remark;
			std::string order_complaint_notify;
		};

	public:
		WXComplaintComponent();
		~WXComplaintComponent() override = default;
	public:
		int OnComplaint(const json::r::Document & document);
		int FindInfo(const std::string & id, json::w::Document & response);
	private:
		bool Awake() final;
		bool LateAwake() final;
	private:
		LangText mText;
		class WeChatComponent * mWeChat;
	};
}


#endif //APP_WXCOMPLAINTCOMPONENT_H

#endif