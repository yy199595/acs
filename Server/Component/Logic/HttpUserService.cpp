#include"HttpUserService.h"
#include"App/App.h"
#include"Util/MD5.h"
#include"Util/MathHelper.h"
#include"Json/JsonWriter.h"
#include"Component/Redis/RedisComponent.h"
#include"Component/Mysql//MysqlProxyComponent.h"

namespace Sentry
{
	void HttpUserService::Awake()
	{
		this->mRedisComponent = nullptr;
		this->mMysqlComponent = nullptr;
	}

	bool HttpUserService::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->mRedisComponent = this->GetComponent<RedisComponent>());
		LOG_CHECK_RET_FALSE(this->mMysqlComponent = this->GetComponent<MysqlProxyComponent>());
		return HttpService::LateAwake();
	}

	bool HttpUserService::OnInitService(HttpServiceRegister& serviceRegister)
	{
		serviceRegister.Bind("Login", &HttpUserService::Login);
		serviceRegister.Bind("Register", &HttpUserService::Register);
		return true;
	}

	XCode HttpUserService::Login(const Json::Reader& request, Json::Writer& response)
	{
		string account, password;
		LOGIC_THROW_ERROR(request.GetMember("account", account));
		LOGIC_THROW_ERROR(request.GetMember("password", password));

		Json::Writer json;
		json.AddMember("account", account);
		std::string whereJson = json.ToJsonString();
		std::shared_ptr<db_account::tab_user_account> userData = this->mMysqlComponent->QueryOnce<db_account::tab_user_account>(whereJson);
		if (userData == nullptr || userData->password() != password)
		{
			return XCode::Failure;
		}
		Json::Writer jsonWriter;
		std::string newToken = this->NewToken(account);
		jsonWriter.AddMember("token", newToken);
		jsonWriter.AddMember("last_login_time", Helper::Time::GetNowSecTime());

		string updateJson = jsonWriter.ToJsonString();
		db_account::tab_user_account userAccountInfo;
		this->mMysqlComponent->Update<db_account::tab_user_account>(updateJson, whereJson);

		userAccountInfo.set_token(newToken);
		userAccountInfo.set_account(account);
		userAccountInfo.set_last_login_time(Helper::Time::GetNowSecTime());
		XCode code = this->mMysqlComponent->Save(userAccountInfo);
		if (code != XCode::Successful)
		{
			return code;
		}
		s2s::AddToGate_Request gateRequest;
		gateRequest.set_user_id(userAccountInfo.user_id());
		return XCode::Successful;
	}

	XCode HttpUserService::Register(const Json::Reader& request, Json::Writer& response)
	{
		long long phoneNumber = 0;
		string user_account, user_password;
		LOGIC_THROW_ERROR(request.GetMember("account", user_account));
		LOGIC_THROW_ERROR(request.GetMember("password", user_password));
		LOGIC_THROW_ERROR(request.GetMember("phone_num", phoneNumber));
		this->mRedisComponent = this->GetComponent<RedisComponent>();
		std::shared_ptr<RedisResponse> resp = this->mRedisComponent->Call("Account.AddNewUser", user_account);
		if (resp->GetNumber() == 0)
		{
			return XCode::AccountAlreadyExists;
		}
		db_account::tab_user_account userAccountInfo;

		userAccountInfo.set_account(user_account);
		userAccountInfo.set_phone_num(phoneNumber);
		userAccountInfo.set_password(user_password);
		userAccountInfo.set_user_id(resp->GetNumber());
		userAccountInfo.set_register_time(Helper::Time::GetNowSecTime());
		return this->mMysqlComponent->Add(userAccountInfo);
	}

	const std::string HttpUserService::NewToken(const std::string& account)
	{
		char buffer[100] = { 0 };
		int number = Helper::Math::Random<int>();
		long long now = Helper::Time::GetNowSecTime();
		return Helper::Md5::GetMd5(fmt::format("{0}:{1}:{2}", account, now, number));
	}
}// namespace Sentry