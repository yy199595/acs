//
// Created by yjz on 2022/4/17.
//

#include"UserSubService.h"
#include"Component/RpcService/LocalServiceComponent.h"
namespace Sentry
{
	bool UserSubService::OnInitService(SubServiceRegister& methodRegister)
	{
		methodRegister.Bind("AddUser", &UserSubService::AddUser);
		methodRegister.Bind("DelUser", &UserSubService::DelUser);
		return true;
	}

	void UserSubService::DelUser(const Json::Reader& jsonReader)
	{
		long long userId = 0;
		std::string service = "";
		jsonReader.GetMember("user_id", userId);
		jsonReader.GetMember("service", service);
		LocalServiceComponent * localServerRpc = this->GetComponent<LocalServiceComponent>(service);
		if(localServerRpc != nullptr)
		{
			localServerRpc->DelEntity(userId);
		}
	}

	void UserSubService::AddUser(const Json::Reader& jsonReader)
	{
		long long userId = 0;
		std::string address = "";
		std::string service = "";
		jsonReader.GetMember("user_id", userId);
		jsonReader.GetMember("address", address);
		jsonReader.GetMember("service", service);
		LocalServiceComponent * localServerRpc = this->GetComponent<LocalServiceComponent>(service);
		if(localServerRpc != nullptr)
		{
			localServerRpc->AddEntity(userId, address);
		}
	}
}
