﻿#include"HttpUserService.h"
#include"App/App.h"
#include"Util/MD5.h"
#include"Util/MathHelper.h"
#include"Json/JsonWriter.h"
#include"Service/ServiceProxy.h"
#include"Component/Redis/RedisComponent.h"
#include"Component/Mysql//MysqlProxyComponent.h"
#include"Component/Scene/ServiceMgrComponent.h"

namespace Sentry
{
    bool HttpUserService::Awake()
    {
        this->mRedisComponent = nullptr;
        this->mMysqlComponent = nullptr;
        BIND_HTTP_FUNCTION(HttpUserService::Login);
        BIND_HTTP_FUNCTION(HttpUserService::Register);
		return true;
    }

    bool HttpUserService::LateAwake()
    {
        LOG_CHECK_RET_FALSE(this->mRedisComponent = this->GetComponent<RedisComponent>());
        LOG_CHECK_RET_FALSE(this->mMysqlComponent = this->GetComponent<MysqlProxyComponent>());
        auto serviceComponent = this->GetComponent<ServiceMgrComponent>();
        LOG_CHECK_RET_FALSE(this->mGateService = serviceComponent->GetServiceProxy("GateService"));
        return true;
    }

    XCode HttpUserService::Login(const Json::Reader &request, Json::Writer &response)
    {
        string account, password;
        LOGIC_THROW_ERROR(request.GetMember("account", account));
        LOGIC_THROW_ERROR(request.GetMember("password", password));

		Json::Writer json;
		json.AddMember("account", account);
        auto userData = this->mMysqlComponent->QueryOnce<db_account::tab_user_account>(json.ToJsonString());
        if(userData == nullptr || userData->password() != password)
        {
            return XCode::Failure;
        }
		Json::Writer jsonWriter;
        std::string newToken = this->NewToken(account);
		jsonWriter.AddMember("token", newToken);
		jsonWriter.AddMember("last_login_time", Helper::Time::GetSecTimeStamp());

		string updateJson = jsonWriter.ToJsonString();
		db_account::tab_user_account userAccountInfo;

		//this->mMysqlComponent->Update<db_account::tab_user_account>()

		userAccountInfo.set_token(newToken);
		userAccountInfo.set_account(account);
		userAccountInfo.set_last_login_time(Helper::Time::GetSecTimeStamp());
        XCode code = this->mMysqlComponent->Save(userAccountInfo);
        if(code != XCode::Successful)
        {
            return code;
        }
        s2s::AddToGate_Request gateRequest;
        gateRequest.set_user_id(userAccountInfo.user_id());
        const std::string address = this->mGateService->AllotAddress();
        auto allotResponse = this->mGateService->Call("GateService.Allot", gateRequest);
        if(allotResponse->AwaitCode() != XCode::Successful)
        {
            return allotResponse->AwaitCode();
        }
        auto responseData = allotResponse->AwaitData<s2s::AddToGate_Response>();

        response.AddMember("gate_ip", responseData->gate_ip());
        response.AddMember("token", responseData->login_token());
        response.AddMember("gate_port", responseData->gate_port());
        return XCode::Successful;
    }

    XCode HttpUserService::Register(const Json::Reader &request, Json::Writer &response)
    {
        long long phoneNumber = 0;
        string user_account, user_password;
        LOGIC_THROW_ERROR(request.GetMember("account", user_account));
        LOGIC_THROW_ERROR(request.GetMember("password", user_password));
        LOGIC_THROW_ERROR(request.GetMember("phone_num", phoneNumber));
        auto resp = this->mRedisComponent->Call("Account.AddNewUser", user_account);
        if(resp->GetNumber() == 0)
        {
            return XCode::AccountAlreadyExists;
        }
		db_account::tab_user_account userAccountInfo;

		userAccountInfo.set_account(user_account);
		userAccountInfo.set_phone_num(phoneNumber);
		userAccountInfo.set_password(user_password);
		userAccountInfo.set_user_id(resp->GetNumber());
		userAccountInfo.set_register_time(Helper::Time::GetSecTimeStamp());
        return this->mMysqlComponent->Add(userAccountInfo);
    }

    const std::string HttpUserService::NewToken(const std::string & account)
    {
        char buffer[100] = {0};
        int number = Helper::Math::Random<int>();
        long long now = Helper::Time::GetSecTimeStamp();
        return Helper::Md5::GetMd5(fmt::format("{0}:{1}:{2}", account, now, number));
    }
}// namespace Sentry