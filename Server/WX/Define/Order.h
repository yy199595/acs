//
// Created by leyi on 2024/4/8.
//

#ifndef APP_ORDER_H
#define APP_ORDER_H
#include <string>
namespace wx
{
	struct OrderSimpleInfo
	{
		std::string OrderIcon;	//图标
		std::string desc; //必填
		int price = 0; //支付金额
		int amount = 0; //总金额
		int status = 0;
		int userId = 0; //付款人必填
		int target_id = 0; //目标收款人id
		std::string orderId;
		int club_id = 0; //商户id

		int commission = 0; //佣金
		int inviter_id = 0; //邀请人id
		int logistics_type = 3; //物流模式，发货方式枚举值：1、实体物流配送采用快递公司进行实体物流配送形式 2、同城配送 3、虚拟商品，虚拟商品，例如话费充值，点卡等，无实体配送形式 4、用户自提
	};

	enum class price_type
	{
		normal, //普通价
		vip, //会员价
		dou, //双人价
		dirve //自驾价
	};

	struct OrderInfo : public OrderSimpleInfo
	{
		int type = 0;
		int city = 0;
		int productId = 0; //产品id
		std::string openId; //必填
		std::string PrepayId; //预支付id
		long long create_time = 0;
		int refund_price = 0; //退款金额
		std::string custom; //备注信息
		int price_type = 0; //价格类型
		long long over_time = 0; //订单结束时间
		long long refund_time = 0; //可退款时间
		long long sallet_time = 0; //结算时间
		long long exp_time = 0; //	最后退还时间
	};


	class IOrder
	{
	public:
		virtual int GetOrderType() = 0; //订单类型
		virtual int OnOrderCreate(wx::OrderInfo & orderInfo) = 0; //创建订单
		virtual int OnOrderPayDone(wx::OrderInfo & orderInfo) = 0; //支付完成
		virtual int OnOrderRefund(wx::OrderInfo & orderInfo) { return 1; }; //订单退款
		virtual void OnRequestRefund(wx::OrderInfo & orderInfo) { } //申请退款事件
	};

	namespace order_type
	{
		constexpr int vip = 1;
		constexpr int activity = 2;
	}
}

#endif //APP_ORDER_H
