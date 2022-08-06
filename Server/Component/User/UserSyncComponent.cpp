//
// Created by yjz on 2022/5/15.
//

#include"UserSyncComponent.h"
#include"Component/Redis/MainRedisComponent.h"
#include"Component/RpcService/LocalService.h"
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
		jsonWrite << "account" << account;
		std::shared_ptr<Json::Reader> response(new Json::Reader());
		if(!this->mRedisComponent->Call("main", "user.add_user", jsonWrite, response))
		{
			return 0;
		}
		long long userId = 0;
		return response->GetMember("user_id", userId) ? userId : 0;
	}

	bool UserSyncComponent::SetToken(const std::string& token, long long userId, int time)
	{
		Json::Writer jsonWrite;
        std::shared_ptr<Json::Reader> response(new Json::Reader());
		jsonWrite << "time" << time << "token" << token << "user_id" << userId;
		if(!this->mRedisComponent->Call("main", "user.set_token", jsonWrite, response))
        {
            return false;
        }
        bool res = false;
        return response->GetMember("res", res) && res;
	}

	long long UserSyncComponent::GetUserId(const std::string& token)
	{
		Json::Writer jsonWrite;
		jsonWrite << "token" << token;
		std::shared_ptr<Json::Reader> response(new Json::Reader());
		if(!this->mRedisComponent->Call("main", "user.get_token", jsonWrite, response))
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
		jsonWrite << "user_id", userId;
		std::shared_ptr<Json::Reader> response(new Json::Reader());
		if(!this->mRedisComponent->Call("main", "user.get_state", jsonWrite, response))
		{
			return -1;
		}
		int state = 0;
		return response->GetMember("state", state) ? state : -1;
	}

	bool UserSyncComponent::SetUserState(long long userId, int state)
	{
		Json::Writer jsonWrite;
		jsonWrite << "state" << state << "user_id" << userId;
        std::shared_ptr<Json::Reader> response(new Json::Reader());
		return this->mRedisComponent->Call("main", "user.set_state", jsonWrite, response);
	}

	const std::string UserSyncComponent::GetAddress(long long userId, const std::string& service)
	{
		std::string address;
		Json::Writer jsonWrite;
		jsonWrite << "user_id" << userId << "service" << service;
		std::shared_ptr<Json::Reader> response(new Json::Reader());
		if(this->mRedisComponent->Call("main", "user.get_address", jsonWrite, response)
			&& response->GetMember("address", address))
		{

		}
		return address;
	}

	bool UserSyncComponent::SetAddress(long long userId, const std::string& service, const std::string& address, bool broadcast)
	{
		Json::Writer jsonWrite;
		jsonWrite << "user_id" << userId << "service" << service;
		jsonWrite << "address" << address << "broadcast" << broadcast;
        std::shared_ptr<Json::Reader> response(new Json::Reader());
        return this->mRedisComponent->Call("main", "user.set_address", jsonWrite, response);
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
		Service * component = this->GetComponent<Service>(service);
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
			Service * component = this->GetComponent<Service>(service);
			if(component != nullptr && component->GetAddressProxy().DelUserAddress(userId))
			{
				LOG_INFO("remove " << userId << " form " << service);
			}
		}
		return true;
	}
}
