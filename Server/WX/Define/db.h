//
// Created by leyi on 2024/1/10.
//

#ifndef APP_DB_H
#define APP_DB_H

namespace mongo_tab
{
	constexpr const char * ADMIN_LIST = "admin_list"; //后台账号
	constexpr const char * CITY_LIST = "city_list"; //城市列表
	constexpr const char * CITY_USER_LIST = "city_user_list"; //城市主理人列表

	constexpr const char * ORDER_LIST = "order_list"; //订单列表
	constexpr const char * WAIT_ORDER_LIST = "wait_order_list"; //待支付订单列表

	constexpr const char * VIP_CARD_LIST = "vip_card_list"; //vip卡配置界面
	constexpr const char * ACTIVITY_LIST = "activity_list"; //正在进行活动列表
	constexpr const char * USER_INFO_LIST = "user_info_list"; //用户信息列表

	//constexpr const char * ACTIVITY_ATTEND_LIST = "activity_attend_list"; //活动报名信息

	constexpr const char * CLUB_LIST = "club_list"; //俱乐部列表
	constexpr const char * CLUB_MEMBER_LIST = "club_member_list"; //俱乐部成员列表

	constexpr const char * USER_WALLET = "user_wallet"; //用户钱包

	constexpr const char * COUPON_LIST = "coupon_list"; //优惠卷列表
	constexpr const char * USER_COUPON_LIST = "user_coupon_list"; //用户惠卷列表

	constexpr const char * SHARE_VIP_LIT = "share_vip_list"; //分享出去的链接列表

	constexpr const char * ACTIVITY_DATA_LIST = "activity_data_list"; //活动数据表

	constexpr const char * COMPLAINT_INFO_LIST = "complaint_info_list"; //投诉列表
}

namespace config
{
	constexpr int MAX_AMOUNT = 500 * 100;
	constexpr int SALLET_TIME = 0;//86400 * 3; //结算时间
	constexpr float SALLET_RATE = 0.04; //结算费率
	constexpr float COMMISSION_SALLET_RATE = 0.01; //佣金结算
}

namespace redis_tab
{
	constexpr const char * ACTIVITY_USERS = "activity_users";
}

#endif //APP_DB_H
