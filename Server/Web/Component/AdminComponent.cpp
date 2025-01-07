//
// Created by leyi on 2024/2/28.
//

#include "AdminComponent.h"
#include "WX/Define/db.h"
#include "XCode/XCode.h"
#include "Util/Tools/TimeHelper.h"
#include "Http/Client/Http.h"
#include "Server/Config/ServerConfig.h"
#include "Mongo/Component/MongoComponent.h"
namespace acs
{
	AdminComponent::AdminComponent()
	{
		this->mMongo = nullptr;
	}

	bool AdminComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->mMongo = this->GetComponent<MongoComponent>())
		return true;
	}

	void AdminComponent::Complete()
	{
		admin::UserInfo userInfo;
		{
			userInfo.name = "乐意";
			userInfo.account = "yjz";
			userInfo.password = "199595yjz.";
			userInfo.permission = http::PermissAdmin;
		}
		if(this->InsertUser(userInfo) == XCode::Ok)
		{
			json::w::Document updater;
			json::w::Document filter;
			filter.Add("_id", userInfo.account);
			updater.Add("permission", http::PermissAdmin);
			this->mMongo->Update(mongo_tab::ADMIN_LIST, filter, updater);
		}
		this->mMongo->SetIndex(mongo_tab::ADMIN_LIST, "user_id", 1, true);
	}

	int AdminComponent::UpdateUser(int userId, json::w::Document& document) const
	{
		json::w::Document filter;
		filter.Add("user_id", userId);
		return this->mMongo->Update(mongo_tab::ADMIN_LIST, filter, document);
	}

	int AdminComponent::Remove(int userId) const
	{
		json::w::Document filter;
		filter.Add("user_id", userId);
		return this->mMongo->Remove(mongo_tab::ADMIN_LIST, filter, 1);
	}

	int AdminComponent::List(int page, json::w::Document& response) const
	{
		json::w::Document filter;
		db::mongo::find_page::request request;
		auto jsonArray = response.AddArray("list");
		int count = this->mMongo->Count(mongo_tab::ADMIN_LIST, filter);
		std::unique_ptr<db::mongo::find_page::response> response1 = std::make_unique<db::mongo::find_page::response>();
		{
			request.set_count(10);
			request.set_page(page);

			request.add_fields("_id");
			request.add_fields("name");
			request.add_fields("user_id");
			request.add_fields("login_ip");
			request.add_fields("permission");
			request.add_fields("login_time");
			request.add_fields("create_time");
			request.set_tab(mongo_tab::ADMIN_LIST);
			if (this->mMongo->FindPage(request, response1.get()) != XCode::Ok)
			{
				response.Add("count", 0);
				return XCode::Failure;
			}
		}
		response.Add("count", count);
		for (int index = 0; index < response1->json_size(); index++)
		{
			jsonArray->PushJson(response1->json(index));
		}
		return XCode::Ok;
	}

	int AdminComponent::InsertUser(admin::UserInfo & userInfo) const
	{
		json::w::Document document;
		userInfo.user_id = this->mMongo->Inc("admin_id");
		if(userInfo.user_id <= 0)
		{
			return XCode::Failure;
		}
		long long nowTime = help::Time::NowSec();
		{
			userInfo.login_time = nowTime;
			userInfo.create_time = nowTime;
		}
		AdminComponent::Encode(userInfo, document);
		return this->mMongo->Insert(mongo_tab::ADMIN_LIST, document);
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
		return AdminComponent::Decode(*response);
	}

	std::unique_ptr<admin::UserInfo> AdminComponent::GetUserInfo(const std::string& account) const
	{
		json::w::Document filter;
		filter.Add("_id", account);
		std::unique_ptr<json::r::Document> response = std::make_unique<json::r::Document>();
		if(this->mMongo->FindOne(mongo_tab::ADMIN_LIST, filter, response.get()) != XCode::Ok)
		{
			return nullptr;
		}
		return AdminComponent::Decode(*response);
	}

	void AdminComponent::Encode(const admin::UserInfo& userInfo, json::w::Document& document)
	{
		document.Add("name", userInfo.name);
		document.Add("_id", userInfo.account);
		document.Add("user_id", userInfo.user_id);
		document.Add("password", userInfo.password);
		document.Add("login_ip", userInfo.login_ip);
		document.Add("login_time", userInfo.login_time);
		document.Add("create_time", userInfo.create_time);
		document.Add("permission", userInfo.permission);
	}

	std::unique_ptr<admin::UserInfo> AdminComponent::Decode(json::r::Document& document)
	{
		std::unique_ptr<admin::UserInfo> userInfo = std::make_unique<admin::UserInfo>();
		{
			document.Get("name", userInfo->name);
			document.Get("_id", userInfo->account);
			document.Get("user_id", userInfo->user_id);
			document.Get("password", userInfo->password);
			document.Get("login_ip", userInfo->login_ip);
			document.Get("login_time", userInfo->login_time);
			document.Get("create_time", userInfo->create_time);
			document.Get("permission", userInfo->permission);
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
		return acs::AdminComponent::Decode(document);
	}
}