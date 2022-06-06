#include"HttpUserService.h"
#include"App/App.h"
#include"Util/MD5.h"
#include"Json/JsonWriter.h"
#include"Component/Gate/GateService.h"
#include"Component/Redis/MainRedisComponent.h"
#include"Component/Mysql/MysqlAgentComponent.h"

namespace Sentry
{
	void HttpUserService::Awake()
	{
		this->mMysqlComponent = nullptr;
		this->mUserSyncComponent = nullptr;
	}

	bool HttpUserService::LateAwake()
	{
		LOG_CHECK_RET_FALSE(LocalHttpService::LateAwake());
		this->mGateService = this->GetComponent<GateService>();
		this->mMysqlComponent = this->GetComponent<MysqlAgentComponent>();
		this->mUserSyncComponent = this->GetComponent<UserSyncComponent>();
		return true;
	}

	bool HttpUserService::OnStartService(HttpServiceRegister& serviceRegister)
	{
		serviceRegister.Bind("Login", &HttpUserService::Login);
		serviceRegister.Bind("Register", &HttpUserService::Register);
		LOG_CHECK_RET_FALSE(this->mMysqlComponent = this->GetComponent<MysqlAgentComponent>());
		return true;
	}

	void HttpUserService::OnAllServiceStart()
	{

	}

	XCode HttpUserService::Login(const HttpHandlerRequest& request, HttpHandlerResponse& response)
	{
		std::string account, password;
		Json::Reader jsonRead(request.GetContent());
		LOGIC_THROW_ERROR(jsonRead.GetMember("account", account));
		LOGIC_THROW_ERROR(jsonRead.GetMember("password", password));

		Json::Writer queryJson;
		queryJson.AddMember("account", account);
		const std::string whereJson = queryJson.ToJsonString();
		std::shared_ptr<db_account::tab_user_account> userAccount =
			std::make_shared<db_account::tab_user_account>();
		XCode code = this->mMysqlComponent->QueryOnce(whereJson, userAccount);
		if (code != XCode::Successful)
		{
			response.AddHead("error", "query user data error");
			return code;
		}

		if (userAccount->password() != password)
		{
			LOG_ERROR(account << " login failure password error");
			response.AddHead("error", "user password error");
			return XCode::Failure;
		}
		std::string newToken;
		Json::Writer jsonWriter;
		this->NewToken(account, newToken);
		jsonWriter.AddMember("token", newToken);
		jsonWriter.AddMember("last_login_time", Helper::Time::GetNowSecTime());
		this->mMysqlComponent->Update<db_account::tab_user_account>(jsonWriter.ToJsonString(), whereJson);

		std::string address;
		if (!this->mGateService->GetAddressProxy().GetAddress(address))
		{
			return XCode::AddressAllotFailure;
		}
		std::shared_ptr<com::Type::String> gateAddress(new com::Type::String());
		if (this->mGateService->Call(address, "QueryListener", gateAddress) != XCode::Successful)
		{
			return XCode::AddressAllotFailure;
		}
		if(this->mUserSyncComponent->SetToken(newToken, userAccount->user_id(), 30))
		{
			Json::Writer jsonResponse;
			jsonResponse.AddMember("token", newToken);
			jsonResponse.AddMember("address", gateAddress->str());
			response.WriteString(jsonResponse.ToJsonString());
			return XCode::Successful;
		}
		return XCode::RedisSocketError;
	}

	XCode HttpUserService::Register(const HttpHandlerRequest& request, HttpHandlerResponse& response)
	{
		long long phoneNumber = 0;
		string user_account, user_password;
		Json::Reader jsonRead(request.GetContent());
		LOGIC_THROW_ERROR(jsonRead.GetMember("account", user_account));
		LOGIC_THROW_ERROR(jsonRead.GetMember("password", user_password));
		LOGIC_THROW_ERROR(jsonRead.GetMember("phone_num", phoneNumber));
		long long userId = this->mUserSyncComponent->AddNewUser(user_account);
		LOG_DEBUG(user_account << " start register ....");
		if(userId == 0)
		{
			response.AddHead("error", "the user already exists");
			return XCode::RedisSocketError;
		}
		long long nowTime = Helper::Time::GetNowSecTime();
		db_account::tab_user_account userAccountInfo;

		userAccountInfo.set_user_id(userId);
		userAccountInfo.set_register_time(nowTime);
		userAccountInfo.set_account(user_account);
		userAccountInfo.set_phone_num(phoneNumber);
		userAccountInfo.set_password(user_password);
		return this->mMysqlComponent->Add(userAccountInfo);
	}

	void HttpUserService::NewToken(const std::string& account, std::string & token)
	{
		std::string nowTime = Helper::Time::GetDateString();
		token = Helper::Md5::GetMd5(fmt::format("{0}:{1}", account, nowTime));
	}

}// namespace Sentry