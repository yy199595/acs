//
// Created by 64658 on 2025/4/23.
//

#include "MongoReadProxy.h"
#include "Mongo/Client/MongoProto.h"
#include "Mongo/Component/MongoDBComponent.h"
namespace acs
{
	MongoReadProxy::MongoReadProxy()
	{
		this->mMongo = nullptr;
	}

	bool MongoReadProxy::OnInit()
	{
		BIND_RPC_METHOD(MongoReadProxy::Find);
		BIND_RPC_METHOD(MongoReadProxy::Count);
		BIND_RPC_METHOD(MongoReadProxy::GetMore);
		BIND_RPC_METHOD(MongoReadProxy::FindOne);
		BIND_RPC_METHOD(MongoReadProxy::FindPage);
		LOG_CHECK_RET_FALSE(this->mMongo = this->GetComponent<MongoDBComponent>())
		return true;
	}

	int MongoReadProxy::Find(const json::r::Document& request, json::w::Document& response)
	{
		int limit = 0;
		std::string tab;
		std::unique_ptr<json::r::Value> filter;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("filter", filter) && filter->IsObject())
		std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
		{
			bson::Writer::Document filterDocument;
			if (!filterDocument.FromByJson(*filter))
			{
				return XCode::CallArgsError;
			}
			mongoRequest->GetCollection("find", tab).Filter(filterDocument);
			if (request.Get("limit", limit))
			{
				mongoRequest->Limit(limit);
			}
			std::unique_ptr<json::r::Value> fields;
			if (request.Get("fields", fields))
			{
				std::string field;
				bson::Writer::Document mode;
				for (size_t index = 0; index < fields->MemberCount(); index++)
				{
					LOG_ERROR_CHECK_ARGS(fields->Get(index, field));
					mode.Add(field.c_str(), 1);
				}
				mongoRequest->document.Add("projection", mode);
			}
		}

		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(std::move(mongoRequest));
		if (mongoResponse == nullptr || mongoResponse->GetResults().empty())
		{
			return XCode::NotFoundData;
		}
		response.Add("cursor", mongoResponse->GetCursor());
		std::unique_ptr<json::w::Value> jsonArray = response.AddArray("list");
		for (const std::string& json: mongoResponse->GetResults())
		{
			jsonArray->PushObject(json);
		}
		return XCode::Ok;
	}

	int MongoReadProxy::FindOne(const json::r::Document& request, json::w::Document& response)
	{
		std::string tab;
		std::unique_ptr<json::r::Value> filter;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("filter", filter) && filter->IsObject())
		std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
		{
			bson::Writer::Document filterDocument;
			if(!filterDocument.FromByJson(*filter))
			{
				return XCode::CallArgsError;
			}
			mongoRequest->GetCollection("find", tab).Filter(filterDocument).Limit(1);

			std::unique_ptr<json::r::Value> fields;
			if(request.Get("fields", fields))
			{
				std::string field;
				bson::Writer::Document mode;
				for(size_t index = 0; index < fields->MemberCount(); index++)
				{
					LOG_ERROR_CHECK_ARGS(fields->Get(index, field));
					mode.Add(field.c_str(), 1);
				}
				mongoRequest->document.Add("projection", mode);
			}
		}

		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(std::move(mongoRequest));
		if (mongoResponse == nullptr || mongoResponse->GetResults().empty())
		{
			return XCode::NotFoundData;
		}
		response.AddObject("data", mongoResponse->GetResults().front());
		return XCode::Ok;
	}

	int MongoReadProxy::FindPage(const json::r::Document& request, json::w::Document& response)
	{
		int page = 1;
		int limit = 10;
		std::string tab;
		request.Get("limit", limit);
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("page", page))
		std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
		{
			const int skip = (page - 1) * limit;
			std::unique_ptr<json::r::Value> filter;
			mongoRequest->GetCollection("find", tab);
			if (request.Get("filter", filter) && filter->IsObject())
			{
				bson::Writer::Document document;
				if (!document.FromByJson(*filter))
				{
					return XCode::CallArgsError;
				}
				mongoRequest->Filter(document);
			}
			mongoRequest->Skip(skip).Limit(limit);
			std::unique_ptr<json::r::Value> sorter;
			if (request.Get("sort", sorter) && sorter->IsObject())
			{
				bson::Writer::Document sortDocument;
				if (!sortDocument.FromByJson(*sorter))
				{
					return XCode::CallArgsError;
				}
				mongoRequest->document.Add("sort", sortDocument);
			}

			std::unique_ptr<json::r::Value> fields;
			if(request.Get("fields", fields))
			{
				std::string field;
				bson::Writer::Document mode;
				for(size_t index = 0; index < fields->MemberCount(); index++)
				{
					LOG_ERROR_CHECK_ARGS(fields->Get(index, field));
					mode.Add(field.c_str(), 1);
				}
				mongoRequest->document.Add("projection", mode);
			}
		}
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(std::move(mongoRequest));
		if (mongoResponse == nullptr || mongoResponse->GetResults().empty())
		{
			return XCode::NotFoundData;
		}
		std::unique_ptr<json::w::Value> jsonArray = response.AddArray("list");
		for (const std::string& json: mongoResponse->GetResults())
		{
			jsonArray->PushObject(json);
		}
		return XCode::Ok;
	}

	int MongoReadProxy::Count(const json::r::Document& request, json::w::Document& response)
	{
		std::string tab;
		std::unique_ptr<json::r::Value> filter;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
		{
			bson::Writer::Document filterDocument;
			mongoRequest->GetCollection("count", tab);
			if (request.Get("filter", filter) && filter->IsObject() && filterDocument.FromByJson(*filter))
			{
				mongoRequest->Query(filterDocument);
			}
		}
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(std::move(mongoRequest));
		if (mongoResponse == nullptr)
		{
			return XCode::Failure;
		}

		int count = 0;
		if (!mongoResponse->Document().Get("n", count))
		{
			return XCode::Failure;
		}
		response.Add("count", count);
		return XCode::Ok;
	}

	int MongoReadProxy::GetMore(const json::r::Document& request, json::w::Document& response)
	{
		std::string tab;
		int batchSize = 0;
		long long cursor = 0;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("cursor", cursor))
		std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
		{
			mongoRequest->cmd = "getMore";
			mongoRequest->document.Add("getMore", cursor);
			mongoRequest->GetCollection("collection", tab);
			if(request.Get("batchSize", batchSize))
			{
				mongoRequest->document.Add("batchSize", batchSize);
			}
			std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(std::move(mongoRequest));
			if (mongoResponse == nullptr)
			{
				return XCode::Failure;
			}
			response.Add("cursor", mongoResponse->GetCursor());
			std::unique_ptr<json::w::Value> jsonArray = response.AddArray("list");
			for (const std::string& document: mongoResponse->GetResults())
			{
				jsonArray->PushObject(document);
			}
		}
		return XCode::Ok;
	}
}