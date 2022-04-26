#include"HttpUserService.h"
#include"App/App.h"
#include"Util/MD5.h"
#include"Util/MathHelper.h"
#include"Json/JsonWriter.h"
#include"Component/Gate/GateService.h"
#include"Component/Service/UserSubService.h"
#include"Component/Redis/MainRedisComponent.h"
#include"Component/Mysql/MysqlProxyComponent.h"

namespace Sentry
{
	void HttpUserService::Awake()
	{
		this->mMysqlComponent = nullptr;
		this->mRedisComponent = nullptr;
	}

	bool HttpUserService::LateAwake()
	{
		LOG_CHECK_RET_FALSE(HttpService::LateAwake());
		this->mGateService = this->GetComponent<GateService>();
		LOG_CHECK_RET_FALSE(this->mRedisComponent = this->GetComponent<MainRedisComponent>());
		return true;
	}

	bool HttpUserService::OnInitService(HttpServiceRegister& serviceRegister)
	{
		serviceRegister.Bind("Login", &HttpUserService::Login);
		serviceRegister.Bind("Register", &HttpUserService::Register);
		LOG_CHECK_RET_FALSE(this->mMysqlComponent = this->GetComponent<MysqlProxyComponent>());
		return true;
	}

	XCode HttpUserService::Login(const Json::Reader& request, Json::Writer& response)
	{
		string account, password;
		LOGIC_THROW_ERROR(request.GetMember("account", account));
		LOGIC_THROW_ERROR(request.GetMember("password", password));

		Json::Writer queryJson;
		queryJson.AddMember("account", account);
		const std::string whereJson = queryJson.ToJsonString();
		std::shared_ptr<db_account::tab_user_account> userAccount =
			std::make_shared<db_account::tab_user_account>();
		XCode code = this->mMysqlComponent->QueryOnce(whereJson, userAccount);
		if (code != XCode::Successful)
		{
			return code;
		}

		if (userAccount->password() != password)
		{
			LOG_ERROR(account << " login failure password error");
			response.AddMember("error", "user password error");
			return XCode::Failure;
		}
		Json::Writer jsonWriter;
		std::string newToken = this->NewToken(account);
		jsonWriter.AddMember("token", newToken);
		jsonWriter.AddMember("last_login_time", Helper::Time::GetNowSecTime());

		string updateJson = jsonWriter.ToJsonString();
		this->mMysqlComponent->Update<db_account::tab_user_account>(updateJson, whereJson);

		std::string address;
		if (!this->mGateService->AllotAddress(address))
		{
			return XCode::AddressAllotFailure;
		}
		s2s::AddressAllot::Request allotRequest;
		allotRequest.set_login_token(newToken);
		allotRequest.set_user_id(userAccount->user_id());
		std::shared_ptr<s2s::AddressAllot::Response> allotResponse(new s2s::AddressAllot::Response());
		if (this->mGateService->Call(address, "Allot", allotRequest, allotResponse) == XCode::Successful)
		{
			if(this->mGateService->AddEntity(userAccount->user_id(), address, true))
			{
				response.AddMember("token", newToken);
				response.AddMember("address", allotResponse->address());
				return XCode::Successful;
			}
		}
		return XCode::AddressAllotFailure;
	}

	XCode HttpUserService::Register(const Json::Reader& request, Json::Writer& response)
	{
		long long phoneNumber = 0;
		string user_account, user_password;
		this->mRedisComponent = this->GetComponent<MainRedisComponent>();
		LOGIC_THROW_ERROR(request.GetMember("account", user_account));
		LOGIC_THROW_ERROR(request.GetMember("password", user_password));
		LOGIC_THROW_ERROR(request.GetMember("phone_num", phoneNumber));

		long long userId = Helper::Guid::Create();
		long long nowTime = Helper::Time::GetNowSecTime();
		db_account::tab_user_account userAccountInfo;

		userAccountInfo.set_user_id(userId);
		userAccountInfo.set_register_time(nowTime);
		userAccountInfo.set_account(user_account);
		userAccountInfo.set_phone_num(phoneNumber);
		userAccountInfo.set_password(user_password);
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