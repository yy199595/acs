//
// Created by leyi on 2024/3/8.
//

#include "RecordComponent.h"
#include "Http/Common/HttpRequest.h"
#include "Http/Common/HttpResponse.h"
#include "Rpc/Config/MethodConfig.h"
#include "Util/Time/TimeHelper.h"
#include "Mongo/Component/MongoComponent.h"

constexpr char * HTTP_RECORD_LIST = "http_record_list";

namespace joke
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

	void RecordComponent::OnRecord(const HttpMethodConfig * config, http::Request* request, http::Response* response)
	{
		json::w::Document document;
		long long nowTime = help::Time::NowSec();
		document.Add("url", config->Path);
		document.Add("desc", config->Desc);
		document.Add("method", config->Type);
		document.Add("time", nowTime);
		if(request->GetBody() != nullptr)
		{
			std::string req = request->GetBody()->ToStr();
			document.Add("request", req);
		}
		else if(request->GetUrl().GetQuery().Size() > 0)
		{
			std::string req = request->GetUrl().GetQuery().ToStr();
			document.Add("request", req);
		}
		if(response->GetBody() != nullptr)
		{
			std::string res = response->GetBody()->ToStr();
			document.Add("response", res);
		}
		int userId = 0;
		if(request->GetUrl().GetQuery().Get(http::query::UserId, userId))
		{
			document.Add("user_id", userId);
		}
		this->mMongo->Insert(HTTP_RECORD_LIST, document, false);
	}
}