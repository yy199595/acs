//
// Created by leyi on 2024/2/22.
//
#ifdef __ENABLE_OPEN_SSL__
#ifndef APP_WECHATCOMPONENT_H
#define APP_WECHATCOMPONENT_H
#include "WX/Config/WxConfig.h"
#include "WX/Define/Order.h"
#include "Entity/Component/Component.h"
namespace wx
{
	struct UserInfo
	{
		std::string openId;
		std::string unionid;
		std::string sessionKey;
	};

	struct PhoneInfo
	{
		std::string countryCode;
		std::string phoneNumber;
		std::string purePhoneNumber;
	};


	struct OrderAmount
	{
		int total;
		int payer_total;
		std::string currency;
		std::string payer_currency;
	};

	constexpr const char * PAY_OK = "SUCCESS";

	struct OrderInfoResponse
	{
		std::string appid;
		std::string mchid;
		std::string openid;
		std::string out_trade_no; //订单号
		std::string transaction_id;
		std::string trade_type;
		std::string trade_state;
		std::string trade_state_desc;
		std::string bank_type;
		std::string attach;
		std::string success_time;
		OrderAmount amount;
	};

	struct RefundAmount
	{
		int total;
		int refund;
	};

	struct RefundResponse
	{
		std::string refund_id; // 微信支付退款号
		std::string out_refund_no; // 商户系统内部的退款单号，商户系统内部唯一，只能是数字、大小写字母_-|*@ ，同一退款单号多次请求只退一笔。
		std::string transaction_id; // 微信支付交易订单号
		std::string out_trade_no; // 原支付交易对应的商户订单号
		std::string channel;		// 枚举值： - ORIGINAL—原路退款 - BALANCE—退回到余额 - OTHER_BALANCE—原账户异常退到其他余额账户 - OTHER_BANKCARD—原银行卡异常退到其他银行卡 * `ORIGINAL` - 原路退款 * `BALANCE` - 退回到余额 * `OTHER_BALANCE` - 原账户异常退到其他余额账户 * `OTHER_BANKCARD` - 原银行卡异常退到其他银行卡
		std::string user_received_account;  	// 取当前退款单的退款入账方，有以下几种情况： 1）退回银行卡：{银行名称}{卡类型}{卡尾号} 2）退回支付用户零钱:支付用户零钱 3）退还商户:商户基本账户商户结算银行账户 4）退回支付用户零钱通:支付用户零钱通
		std::string status;
		RefundAmount amount;
	};

	struct PayMessage
	{
		std::string nonce;
		std::string associated_data;
		std::string ciphertext;
	};

	struct PayResponse
	{
		std::string mchid;
		std::string appid;
		std::string out_trade_no;
		std::string transaction_id;
		std::string trade_state;
		std::string trade_state_desc;
		std::string bank_type;
		std::string attach;
		std::string open_id;
		std::string success_time;
		OrderAmount amount;
	};

	struct TransferDetail
	{
		std::string openid;
		std::string out_detail_no;
		int transfer_amount;
		std::string transfer_remark;
		std::string user_name;
	};

	struct TransferRequest
	{
	public:
		std::string appid;
		std::string out_batch_no;
		std::string batch_name;
		std::string batch_remark;
		std::string transfer_scene_id;
		std::vector<TransferDetail> transfer_detail_list; //数组
	};

	struct TransferResponse
	{
	public:
		std::string batch_id;
		std::string create_time;
		std::string out_batch_no;
		std::string batch_status;
	};

	struct UploadOrder
	{
		std::string transaction_id;
		std::string merchant_trade_no;
		std::string description;
		int paid_amount = 0;
		int order_state = 0; //订单状态枚举：(1) 待发货；(2) 已发货；(3) 确认收货；(4) 交易完成；(5) 已退款。
		bool in_complaint = false; //是否处在交易纠纷中
	};

	struct UploadOrderResponse
	{
		int errcode;
		UploadOrder order;
		std::string errmsg;
	};


	struct AccessToken
	{
		std::string token;
		long long exp_time = 0;
	};

	struct ComplainResource
	{
		std::string algorithm;
		std::string ciphertext;
		std::string original_type;
		std::string associated_data;
		std::string nonce;
	};

	struct ComplainRequest
	{
		std::string id;
		std::string create_time;
		std::string event_type; //通知的类型，投诉事件通知的类型，具体如下：COMPLAINT.CREATE：产生新投诉 COMPLAINT.STATE_CHANGE：投诉状态变化
		std::string summary;
		ComplainResource resource;
	};

	struct ComplainInfo //投诉信息
	{
		std::string action_type;
		std::string complaint_id;
	};

	struct LinkInfo
	{
		std::string errmsg;
		std::string url_link;
	};
}

namespace http
{
	class Request;
	class Response;
	class FromContent;
	class JsonContent;
}

namespace acs
{
	//微信登录，支付组件
	class WeChatComponent final : public Component, public IStart
	{
	public:
		WeChatComponent();
		~WeChatComponent() final = default;
	private:
		bool Awake() final;
		void OnStart() final;
		bool LateAwake() final;
	public:
		bool CreateOrder(wx::OrderInfo * orderInfo);
		bool CloseOrder(const std::string & orderId);
		bool CreateComplaintUrl(const std::string& url = "");
		std::unique_ptr<wx::UserInfo> Login(const std::string & code);
		std::unique_ptr<wx::OrderInfoResponse> GetOrder(const std::string & orderId);
		std::unique_ptr<wx::UploadOrderResponse> GetOrderInfo(const std::string & orderId);
		std::unique_ptr<wx::RefundResponse> Refund(const wx::OrderInfo & orderInfo, const std::string & reason);
		static std::unique_ptr<json::r::Document> DecodeData(const std::string & iv, const std::string & key, const std::string & data);
	public:
		std::unique_ptr<http::Response> GetWxCode(const std::string & path);
		bool OrderSign(const std::string & prepay_id, json::w::Value * document);
		std::unique_ptr<wx::TransferResponse> Transfer(const wx::TransferRequest & request);
	public:
		std::unique_ptr<wx::PayResponse> DecodeResponse(const wx::PayMessage & message) const;
		std::unique_ptr<wx::ComplainInfo> DecodeComplain(const wx::ComplainRequest & message);
		std::unique_ptr<wx::LinkInfo> GetLinkUrl(const std::string & path, const std::string & query);
		std::unique_ptr<wx::LinkInfo> GetSchemeUrl(const std::string & path, const std::string & query);
	public:
		const http::JsonContent * Get(const std::string & url, http::FromContent & fromData);
	public:
		bool SetJumpPath(const std::string & path);
		bool AddTemplate(const std::string & id, const std::string & desc);
	public:
		bool GetAccessToken();
		void ClearAccessToken();
		std::unique_ptr<wx::PhoneInfo> GetPhoneNumByCode(const std::string & code);
		std::unique_ptr<wx::UploadOrderResponse> UploadOrder(const wx::OrderInfo & orderInfo);
	private:
		bool DownCertificates();
		std::unique_ptr<http::Request> NewRequest(const char * method, const std::string & url);
		std::unique_ptr<http::Request> NewRequest(const std::string & url, json::w::Document & body);
	private:
		wx::Config mConfig;
		class HttpComponent * mHttp;
		wx::AccessToken mAccessToken;
	};
}


#endif //APP_WECHATCOMPONENT_H

#endif