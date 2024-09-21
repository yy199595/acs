//
// Created by leyi on 2024/2/28.
//

#ifndef APP_ADMINCOMPONENT_H
#define APP_ADMINCOMPONENT_H

#include "Entity/Component/Component.h"

namespace admin
{
	struct UserInfo
	{
		int user_id = 0; //用户id
		int permission = 1; //权限
		std::string name; //昵称
		std::string account; //账号
		std::string password; //密码
		std::string login_ip; //登录ip
		long long login_time = 0; //登录时间
		long long create_time = 0; //创建时间
	};
}

namespace acs
{
	class AdminComponent : public Component, public IComplete
	{
	public:
		AdminComponent();
		~AdminComponent() final = default;
	public:
		std::unique_ptr<admin::UserInfo> GetUserInfo(int userId);
		std::unique_ptr<admin::UserInfo> GetUserInfo(const std::string & account);
	public:
		int InsertUser(admin::UserInfo & userInfo);
		int UpdateUser(int userId, json::w::Document & document);
	public:
		int Remove(int userId);
		int List(int page, json::w::Document & response);
	private:
		void Complete() final;
		bool LateAwake() final;
		std::unique_ptr<admin::UserInfo> Decode(const std::string & json);
		static std::unique_ptr<admin::UserInfo> Decode(json::r::Document & document);
		static void Encode(const admin::UserInfo & userInfo, json::w::Document & document);
	private:
		class MongoComponent * mMongo;
	};
}


#endif //APP_ADMINCOMPONENT_H
