//
// Created by leyi on 2024/3/8.
//

#include "RecordComponent.h"
#include "Util/Tools/Guid.h"
#include "Http/Common/HttpRequest.h"
#include "Http/Common/HttpResponse.h"
#include "Rpc/Config/MethodConfig.h"
#include "Util/Tools/TimeHelper.h"
#include "Mongo/Component/MongoComponent.h"

constexpr char * HTTP_RECORD_LIST = "http_record_list";

namespace acs
{
	RecordComponent::RecordComponent()
	{
		this->mMongo = nullptr;
	}

	bool RecordComponent::LateAwake()
	{
		this->mMongo = this->GetComponent<MongoComponent>();
		return this->mMongo != nullptr;
	}

	void RecordComponent::OnComplete()
	{
		this->mMongo->SetIndex(HTTP_RECORD_LIST, "time", -1);
	}

	void RecordComponent::OnRequestDone(const HttpMethodConfig & config, const http::Request & request, const http::Response & response)
	{
		if(!config.record)
		{
			return;
		}
		json::w::Document document;
		long long id = help::ID::Create();
		long long nowTime = help::Time::NowSec();
		{
			document.Add("url", config.path);
			document.Add("desc", config.desc);
			document.Add("method", config.type);
			document.Add("time", nowTime);
			document.Add("_id", std::to_string(id));
		}
		if(request.GetBody() != nullptr)
		{
			std::string req = request.GetBody()->ToStr();
			document.Add("request", req);
		}
		else if(request.GetUrl().GetQuery().Size() > 0)
		{
			std::string req = request.GetUrl().GetQuery().ToStr();
			document.Add("request", req);
		}
		if(response.GetBody() != nullptr)
		{
			std::string res = response.GetBody()->ToStr();
			document.Add("response", res);
		}
		int userId = 0;
		if(request.GetUrl().GetQuery().Get(http::query::UserId, userId))
		{
			document.Add("user_id", userId);
		}
		this->mMongo->Insert(HTTP_RECORD_LIST, document, false);
	}
}