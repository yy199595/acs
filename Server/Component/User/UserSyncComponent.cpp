//
// Created by yjz on 2022/5/15.
//

#include"UserSyncComponent.h"
#include"Component/Redis/MainRedisComponent.h"
namespace Sentry
{
	bool UserSyncComponent::LateAwake()
	{
		this->mRedisComponent = this->GetComponent<MainRedisComponent>();
		return true;
	}

	long long UserSyncComponent::AddNewUser(const std::string & account)
	{
		Json::Writer jsonWrite;
		jsonWrite.AddMember("account", account);
		std::shared_ptr<Json::Reader> response(new Json::Reader());
		if(!this->mRedisComponent->CallLua("user.add_user", jsonWrite, response))
		{
			return 0;
		}
		long long userId = 0;
		return response->GetMember("user_id", userId) ? userId : 0;
	}

	bool UserSyncComponent::SetToken(const std::string& token, long long userId, int time)
	{
		Json::Writer jsonWrite;
		jsonWrite.AddMember("time", time);
		jsonWrite.AddMember("token", token);
		jsonWrite.AddMember("user_id", userId);
		return this->mRedisComponent->CallLua("user.set_token", jsonWrite);
	}

	long long UserSyncComponent::GetUserId(const std::string& token)
	{
		Json::Writer jsonWrite;
		jsonWrite.AddMember("token", token);
		std::shared_ptr<Json::Reader> response(new Json::Reader());
		if(!this->mRedisComponent->CallLua("user.get_token", jsonWrite, response))
		{
			LOG_ERROR("gen user by token failure " << token);
			return 0;
		}
		long long userId = 0;
		return response->GetMember("user_id", userId) ? userId : 0;
	}

	int UserSyncComponent::GetUserState(long long userId)
	{
		Json::Writer jsonWrite;
		jsonWrite.AddMember("user_id", userId);
		std::shared_ptr<Json::Reader> response(new Json::Reader());
		if(!this->mRedisComponent->CallLua("user.get_state", jsonWrite))
		{
			return -1;
		}
		int state = 0;
		return response->GetMember("state", state) ? state : -1;
	}

	bool UserSyncComponent::SetUserState(long long userId, int state)
	{
		Json::Writer jsonWrite;
		jsonWrite.AddMember("state", state);
		jsonWrite.AddMember("user_id", userId);
		return this->mRedisComponent->CallLua("user.set_state", jsonWrite);
	}

	const std::string UserSyncComponent::GetAddress(long long userId, const std::string& service)
	{
		std::string address;
		Json::Writer jsonWrite;
		jsonWrite.AddMember("user_id", userId);
		jsonWrite.AddMember("service", service);
		std::shared_ptr<Json::Reader> response(new Json::Reader());
		if(this->mRedisComponent->CallLua("user.get_address", jsonWrite, response)
			&& response->GetMember("address", address))
		{

		}
		return address;
	}

	bool UserSyncComponent::SetAddress(long long userId, const std::string& service, const std::string& address, bool broadcast)
	{
		Json::Writer jsonWrite;
		jsonWrite.AddMember("user_id", userId);
		jsonWrite.AddMember("service", service);
		jsonWrite.AddMember("address", address);
		jsonWrite.AddMember("broadcast", broadcast);
		return this->mRedisComponent->CallLua("user.set_address", jsonWrite);
	}

	bool UserSyncComponent::OnRegisterEvent(NetEventRegistry& eventRegister)
	{
		return eventRegister.Sub("user_join_event", &UserSyncComponent::OnUserJoin, this)
			   && eventRegister.Sub("user_exit_event", &UserSyncComponent::OnUserExit, this);
	}

	bool UserSyncComponent::OnUserJoin(const Json::Reader& json)
	{
		std::string address;
		std::string service;
		long long userId = 0;
		LOG_CHECK_RET_FALSE(json.GetMember("user_id", userId));
		LOG_CHECK_RET_FALSE(json.GetMember("address", address));
		LOG_CHECK_RET_FALSE(json.GetMember("service", service));
		ServiceComponent * component = this->GetComponent<ServiceComponent>(service);
		return component != nullptr && component->GetAddressProxy().AddUserAddress(userId, address);
	}

	bool UserSyncComponent::OnUserExit(const Json::Reader& json)
	{
		long long userId = 0;
		std::vector<std::string> services;
		LOG_CHECK_RET_FALSE(json.GetMember("user_id", userId));
		LOG_CHECK_RET_FALSE(json.GetMember("services", services));
		for(const std::string & service : services)
		{
			ServiceComponent * component = this->GetComponent<ServiceComponent>(service);
			if(component != nullptr && component->GetAddressProxy().DelUserAddress(userId))
			{
				LOG_INFO("remove " << userId << " form " << service);
			}
		}
		return true;
	}
}
