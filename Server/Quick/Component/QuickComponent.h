//
// Created by 64658 on 2024/12/27.
//

#ifndef APP_QUICKCOMPONENT_H
#define APP_QUICKCOMPONENT_H
#include "XML/Document/XDocument.h"
#include "Entity/Component/Component.h"

namespace quick
{
	struct Config
	{
		std::string md5_key;
		std::string callback_key;
	};

	struct PayNotify
	{
		int is_test = 0; //是否为测试订单 1为测试 0为线上正式订单，游戏应根据情况确定上线后是否向测试订单发放道具。
		std::string channel; //渠道标示ID 注意:游戏可根据实情,确定发放道具时是否校验充值来源渠道是否与该角色注册渠道相符
		std::string channel_uid; //渠道用户唯一标示,该值从客户端GetUserId()中可获取
		std::string game_order; //游戏在调用QuickSDK发起支付时传递的游戏方订单,这里会原样传回
		std::string order_no; //QuickSDK唯一订单号
		std::string pay_time; //支付时间 支付时间 2015-01-01 23:00:00
		int amount = 0; //成交金额，单位元，游戏最终发放道具金额应以此为准
		int status = 0; //充值状态:0成功, 1失败(为1时 应返回FAILED失败)
		std::string extras_params; //自定义参数
		std::string original_currency; //海外游戏返回此字段，玩家支付的原始币种
		std::string original_amount; //海外游戏返回此字段，玩家支付的原始币种金额
	};
}


namespace acs
{
	class QuickComponent : public Component
	{
	public:
		QuickComponent();
		~QuickComponent() final = default;
	private:
		bool Awake() final;
		bool LateAwake() final;
	public:
		bool Login(const std::string & uid, const std::string & token);		//登录
		bool Decode(const std::string & message, quick::PayNotify & payNotify); //支付解析
	private:
		bool Decode(const std::string & input, std::string & output);
	private:
		quick::Config mConfig;
		class HttpComponent * mHttp;
	};
}



#endif //APP_QUICKCOMPONENT_H
