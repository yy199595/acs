#include "AccountService.h"
#include "Object/App.h"
#include <Util/MD5.h>
#include <Util/MathHelper.h>
#include "Component/RedisComponent.h"
#include "Component/MysqlProxyComponent.h"
#include"Scene/ServiceMgrComponent.h"
#include"Service/ServiceProxy.h"
#include"MysqlClient/MysqlRpcTaskSource.h"
namespace Sentry
{
    bool AccountService::Awake()
    {
        this->mRedisComponent = nullptr;
        this->mMysqlComponent = nullptr;
        BIND_HTTP_FUNCTION(AccountService::Login);
        BIND_HTTP_FUNCTION(AccountService::Register);
		return true;
    }

    bool AccountService::LateAwake()
    {
        LOG_CHECK_RET_FALSE(this->mRedisComponent = this->GetComponent<RedisComponent>());
        LOG_CHECK_RET_FALSE(this->mMysqlComponent = this->GetComponent<MysqlProxyComponent>());
        auto serviceComponent = this->GetComponent<ServiceMgrComponent>();
        LOG_CHECK_RET_FALSE(this->mGateService = serviceComponent->GetServiceProxy("GateService"));
        return true;
    }

    XCode AccountService::Login(const RapidJsonReader &request, RapidJsonWriter &response)
    {
        string account, password;
        LOG_THROW_ERROR(request.TryGetValue("account", account));
        LOG_THROW_ERROR(request.TryGetValue("password", password));

        this->mTempData.Clear();
        this->mTempData.set_account(account);
        auto taskSource = this->mMysqlComponent->Query(this->mTempData);
        if(taskSource->GetCode() != XCode::Successful)
        {
            return taskSource->GetCode();
        }
        auto userData = taskSource->GetData<db::db_account::tab_user_account>();
        if(userData->password() != password)
        {
            return XCode::Failure;
        }
        std::string newToken = this->NewToken(account);

        this->mTempData.set_token(newToken);
        this->mTempData.set_account(account);
        this->mTempData.set_lastlogin_time(Helper::Time::GetSecTimeStamp());
        XCode code = this->mMysqlComponent->Save(this->mTempData)->GetCode();
        if(code != XCode::Successful)
        {
            return code;
        }
        s2s::AddToGate_Request gateRequest;
        gateRequest.set_user_id(this->mTempData.user_id());
        const std::string address = this->mGateService->AllotAddress();
        auto allotResponse = this->mGateService->Call("GateService.Allot", gateRequest);
        if(allotResponse->AwaitCode() != XCode::Successful)
        {
            return allotResponse->AwaitCode();
        }
        auto responseData = allotResponse->AwaitData<s2s::AddToGate_Response>();

        response.Add("gate_ip", responseData->gate_ip());
        response.Add("token", responseData->login_token());
        response.Add("gate_port", responseData->gate_port());
        return XCode::Successful;
    }

    XCode AccountService::Register(const RapidJsonReader &request, RapidJsonWriter &response)
    {
        long long phoneNumber = 0;
        string user_account, user_password;
        LOG_THROW_ERROR(request.TryGetValue("account", user_account));
        LOG_THROW_ERROR(request.TryGetValue("password", user_password));
        LOG_THROW_ERROR(request.TryGetValue("phone_num", phoneNumber));
        auto resp = this->mRedisComponent->Call("Account.AddNewUser", user_account);
        if(resp->GetNumber() == 0)
        {
            return XCode::AccountAlreadyExists;
        }

        this->mTempData.Clear();
        this->mTempData.set_account(user_account);
        this->mTempData.set_phone_num(phoneNumber);
        this->mTempData.set_password(user_password);
        this->mTempData.set_user_id(resp->GetNumber());
        this->mTempData.set_register_time(Helper::Time::GetSecTimeStamp());
        return this->mMysqlComponent->Add(this->mTempData)->GetCode();
    }

    const std::string AccountService::NewToken(const std::string & account)
    {
        char buffer[100] = {0};
        int number = Helper::Math::Random<int>();
        long long now = Helper::Time::GetSecTimeStamp();
        size_t size = sprintf(buffer, "%s:%lld:%d", account.c_str(), now, number);
        return Helper::Md5::GetMd5(buffer, size);
    }
}// namespace Sentry