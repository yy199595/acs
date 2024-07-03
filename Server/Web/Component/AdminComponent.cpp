//
// Created by leyi on 2024/2/28.
//

#include "AdminComponent.h"
#include "WX/Define/db.h"
#include "XCode/XCode.h"
#include "Util/Time/TimeHelper.h"
#include "Http/Client/Http.h"
#include "Server/Config/ServerConfig.h"
#include "Mongo/Component/MongoComponent.h"
namespace joke
{
	AdminComponent::AdminComponent()
	{
		this->mMongo = nullptr;
	}

	bool AdminComponent::LateAwake()
	{
		this->mMongo = this->GetComponent<MongoComponent>();
		return true;
	}

	void AdminComponent::Complete()
	{
		
	}

	int AdminComponent::UpdateUser(int userId, json::w::Document& document)
	{
		json::w::Document filter;
		filter.Add("user_id", userId);
		return this->mMongo->Update(mongo_tab::ADMIN_LIST, filter, document);
	}


	std::unique_ptr<admin::UserInfo> AdminComponent::GetUserInfo(int userId)
	{
		json::w::Document filter;
		filter.Add("user_id", userId);
		std::unique_ptr<json::r::Document> response = std::make_unique<json::r::Document>();
		if(this->mMongo->FindOne(mongo_tab::ADMIN_LIST, filter, response.get()) != XCode::Ok)
		{
			return nullptr;
		}
		return this->Decode(*response);
	}

	std::unique_ptr<admin::UserInfo> AdminComponent::GetUserInfo(const std::string& account)
	{
		json::w::Document filter;
		filter.Add("_id", account);
		std::unique_ptr<json::r::Document> response = std::make_unique<json::r::Document>();
		if(this->mMongo->FindOne(mongo_tab::ADMIN_LIST, filter, response.get()) != XCode::Ok)
		{
			return nullptr;
		}
		return this->Decode(*response);
	}

	void AdminComponent::Encode(const admin::UserInfo& userInfo, json::w::Document& document)
	{
		document.Add("city", userInfo.city);
		document.Add("name", userInfo.name);
		document.Add("_id", userInfo.account);
		document.Add("user_id", userInfo.user_id);
		document.Add("password", userInfo.password);
		document.Add("login_ip", userInfo.login_ip);
		document.Add("login_time", userInfo.login_time);
		document.Add("create_time", userInfo.create_time);
		document.Add("permission", userInfo.permission);
		document.Add("city_name", userInfo.city_name);
	}

	std::unique_ptr<admin::UserInfo> AdminComponent::Decode(json::r::Document& document)
	{
		std::unique_ptr<admin::UserInfo> userInfo = std::make_unique<admin::UserInfo>();
		{
			document.Get("city", userInfo->city);
			document.Get("name", userInfo->name);
			document.Get("_id", userInfo->account);
			document.Get("user_id", userInfo->user_id);
			document.Get("password", userInfo->password);
			document.Get("login_ip", userInfo->login_ip);
			document.Get("login_time", userInfo->login_time);
			document.Get("create_time", userInfo->create_time);
			document.Get("permission", userInfo->permission);
			document.Get("city_name", userInfo->city_name);
		}
		return userInfo;
	}

	std::unique_ptr<admin::UserInfo> AdminComponent::Decode(const std::string& json)
	{
		json::r::Document document;
		if(!document.Decode(json))
		{
			return nullptr;
		}
		return this->Decode(document);
	}
}