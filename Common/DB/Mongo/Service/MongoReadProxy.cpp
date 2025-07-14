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
		BIND_RPC_METHOD(MongoReadProxy::Sample);
		BIND_RPC_METHOD(MongoReadProxy::GetMore);
		BIND_RPC_METHOD(MongoReadProxy::FindOne);
		BIND_RPC_METHOD(MongoReadProxy::FindPage);
		BIND_RPC_METHOD(MongoReadProxy::Aggregate);
		BIND_RPC_METHOD(MongoReadProxy::ListDatabases);
		BIND_RPC_METHOD(MongoReadProxy::ListCollections);
		LOG_CHECK_RET_FALSE(this->mMongo = this->GetComponent<MongoDBComponent>())
		return true;
	}

	int MongoReadProxy::Find(const json::r::Document& request, json::w::Document& response)
	{
		int limit = 0;
		std::string tab;
		json::r::Value filter;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
		{
			mongoRequest->GetCollection("find", tab);
			if(request.Get("filter", filter) && filter.IsObject())
			{
				LOG_ERROR_CHECK_ARGS(mongoRequest->Filter(filter))
			}

			if (request.Get("limit", limit))
			{
				mongoRequest->Limit(limit);
			}
			json::r::Value fields;
			if (request.Get("fields", fields))
			{
				std::string field;
				bson::w::Document mode;
				for (size_t index = 0; index < fields.MemberCount(); index++)
				{
					LOG_ERROR_CHECK_ARGS(fields.Get(index, field));
					mode.Add(field.c_str(), 1);
				}
				mongoRequest->document.Add("projection", mode);
			}
		}
		std::string req = mongoRequest->ToString();
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(mongoRequest);
		if (mongoResponse == nullptr)
		{
			return XCode::Failure;
		}
		if(mongoResponse->result.empty())
		{
			return XCode::DataBaseFindDataEmpty;
		}
		response.Add("cursor", mongoResponse->GetCursor());
		std::unique_ptr<json::w::Value> jsonArray = response.AddArray("list");
		for (const std::string& json: mongoResponse->result)
		{
			jsonArray->PushObject(json);
		}
		return XCode::Ok;
	}

	int MongoReadProxy::FindOne(const json::r::Document& request, rpc::Message& response)
	{
		std::string tab;
		json::r::Value filter;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("filter", filter) && filter.IsObject())
		std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
		{
			mongoRequest->GetCollection("find", tab);
			LOG_ERROR_CHECK_ARGS(mongoRequest->Filter(filter));
			mongoRequest->Limit(1);

			json::r::Value fields;
			if(request.Get("fields", fields))
			{
				std::string field;
				bson::w::Document mode;
				for(size_t index = 0; index < fields.MemberCount(); index++)
				{
					LOG_ERROR_CHECK_ARGS(fields.Get(index, field));
					mode.Add(field.c_str(), 1);
				}
				mongoRequest->document.Add("projection", mode);
			}
		}

		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(mongoRequest);
		if (mongoResponse == nullptr)
		{
			return XCode::NotFoundData;
		}
		if(mongoResponse->result.empty())
		{
			return XCode::DataBaseFindDataEmpty;
		}
		response.SetContent(rpc::proto::json, mongoResponse->result.front());
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
			json::r::Value filter;
			const int skip = (page - 1) * limit;
			mongoRequest->GetCollection("find", tab);
			if (request.Get("filter", filter) && filter.IsObject())
			{
				bson::w::Document document;
				if (!document.FromByJson(filter))
				{
					return XCode::CallArgsError;
				}
				mongoRequest->Filter(document);
			}
			json::r::Value sorter;
			mongoRequest->Skip(skip).Limit(limit);
			if (request.Get("sort", sorter) && sorter.IsObject())
			{
				bson::w::Document sortDocument;
				if (!sortDocument.FromByJson(sorter))
				{
					return XCode::CallArgsError;
				}
				mongoRequest->document.Add("sort", sortDocument);
			}

			json::r::Value fields;
			if(request.Get("fields", fields) && fields.IsArray())
			{
				size_t count = 0;
				bson::w::Document mode;
				for(size_t index = 0; index < fields.MemberCount(); index++)
				{
					const char * field = fields.GetString(index, count);
					if(field != nullptr && count > 0)
					{
						mode.Add(field, 1);
					}
				}
				mongoRequest->document.Add("projection", mode);
			}
		}
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(mongoRequest);
		if (mongoResponse == nullptr)
		{
			return XCode::Failure;
		}
		if(mongoResponse->result.empty())
		{
			return XCode::DataBaseFindDataEmpty;
		}
		std::unique_ptr<json::w::Value> jsonArray = response.AddArray("list");
		for (const std::string& json: mongoResponse->result)
		{
			jsonArray->PushObject(json);
		}
		return XCode::Ok;
	}

	int MongoReadProxy::Count(const json::r::Document& request, json::w::Document& response)
	{
		std::string tab;
		json::r::Value filter;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
		{
			bson::w::Document filterDocument;
			mongoRequest->GetCollection("count", tab);
			if (request.Get("filter", filter) && filter.IsObject() && filterDocument.FromByJson(filter))
			{
				mongoRequest->Query(filterDocument);
			}
		}
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(mongoRequest);
		if (mongoResponse == nullptr)
		{
			return XCode::Failure;
		}

		int count = 0;
		if (!mongoResponse->document.Get("n", count))
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
			std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(mongoRequest);
			if (mongoResponse == nullptr)
			{
				return XCode::Failure;
			}
			response.Add("cursor", mongoResponse->GetCursor());
			std::unique_ptr<json::w::Value> jsonArray = response.AddArray("list");
			for (const std::string& document: mongoResponse->result)
			{
				jsonArray->PushObject(document);
			}
		}
		return XCode::Ok;
	}

	int MongoReadProxy::Sample(const json::r::Document& request, json::w::Document& response)
	{
		std::string tab;
		unsigned int count = 0;
		unsigned int batchSize = 0;
		request.Get("batchSize", batchSize);
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab));
		LOG_ERROR_CHECK_ARGS(request.Get("count", count) && count > 0);
		std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
		{
			json::r::Value filterValue;
			bson::w::Document pipeline(false);
			mongoRequest->GetCollection("aggregate", tab);
			if(request.Get("filter", filterValue) && filterValue.IsObject())
			{
				bson::w::Document document;
				if(!document.FromByJson(filterValue))
				{
					return XCode::CallArgsError;
				}
				bson::w::Document matchDocument;
				matchDocument.Add("$match", document);
				pipeline.Push(matchDocument);
			}
			{
				bson::w::Document document;
				document.Add("size", count);
				bson::w::Document sampleDocument;
				sampleDocument.Add("$sample", document);
				pipeline.Push(sampleDocument);
			}
			json::r::Value fieldValue;
			if(request.Get("fields", fieldValue) && fieldValue.IsArray())
			{
				bson::w::Document document;
				for(size_t index = 0; index < fieldValue.MemberCount(); index++)
				{
					size_t size = 0;
					const char * key = fieldValue.GetString(index, size);
					if(key != nullptr)
					{
						return XCode::CallArgsError;
					}
					document.Add(key, 1);
				}
			}
			bson::w::Document cursor;
			if(batchSize > 0)
			{
				cursor.Add("batchSize", batchSize);
			}
			mongoRequest->document.Add("cursor", cursor);
			mongoRequest->document.Add("pipeline", pipeline);
			mongoRequest->document.Add("allowDiskUse", true);
		}
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(mongoRequest);
		if (mongoResponse == nullptr)
		{
			return XCode::Failure;
		}
		response.Add("cursor", mongoResponse->GetCursor());
		std::unique_ptr<json::w::Value> jsonArray = response.AddArray("list");
		for (const std::string& document: mongoResponse->result)
		{
			jsonArray->PushObject(document);
		}
		return XCode::Ok;
	}

	int MongoReadProxy::Aggregate(const json::r::Document& request, json::w::Document& response)
	{
		std::string tab;
		json::r::Value pipeline;
		unsigned int batchSize = 0;
		request.Get("batchSize", batchSize);
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab));
		LOG_ERROR_CHECK_ARGS(request.Get("pipeline", pipeline) && pipeline.IsArray());
		std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
		{
			bson::w::Document pipelineArray(false);
			mongoRequest->GetCollection("aggregate", tab);
			for(size_t index = 0; index < pipeline.MemberCount(); index++)
			{
				json::r::Value jsonObject;
				if(!pipeline.Get(index, jsonObject))
				{
					return XCode::CallArgsError;
				}
				bson::w::Document document;
				if(!document.FromByJson(jsonObject))
				{
					return XCode::CallArgsError;
				}
				pipelineArray.Push(document);
			}
			bson::w::Document cursor;
			if(batchSize > 0)
			{
				cursor.Add("batchSize", batchSize);
			}
			mongoRequest->document.Add("cursor", cursor);
			mongoRequest->document.Add("pipeline", pipelineArray);
			mongoRequest->document.Add("allowDiskUse", true);
		}
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(mongoRequest);
		if (mongoResponse == nullptr)
		{
			return XCode::Failure;
		}
		response.Add("cursor", mongoResponse->GetCursor());
		std::unique_ptr<json::w::Value> jsonArray = response.AddArray("list");
		for (const std::string& document: mongoResponse->result)
		{
			jsonArray->PushObject(document);
		}

		return XCode::Ok;
	}

	int MongoReadProxy::ListDatabases(json::w::Document& response)
	{
		std::unique_ptr<mongo::Request> request1 = std::make_unique<mongo::Request>();
		{
			bson::w::Document doc1;
			doc1.Add("nameOnly", false);
			request1->dataBase = "admin";
			request1->document.Add("listDatabases", doc1);
		}
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(request1);
		if (mongoResponse == nullptr)
		{
			return XCode::Failure;
		}
		std::list<bson::r::Document> results;
		const bson::r::Document& readDocument = mongoResponse->document;
		if(readDocument.Get("databases", results))
		{
			return XCode::DataBaseFindDataEmpty;
		}
		std::unique_ptr<json::w::Value> jsonArray = response.AddArray("list");
		for (bson::r::Document& document: results)
		{
			std::string json;
			if(document.WriterToJson(&json))
			{
				jsonArray->PushObject(json);
			}
		}
		return XCode::Ok;
	}

	int MongoReadProxy::ListCollections(const json::r::Document& request, json::w::Document& response)
	{
		std::unique_ptr<mongo::Request> message = std::make_unique<mongo::Request>();
		{
			bson::w::Document doc1;
			doc1.Add("nameOnly", true);
			request.Get("db", message->dataBase);
			message->document.Add("listCollections", doc1);
		}
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(message);
		if (mongoResponse == nullptr)
		{
			return XCode::Failure;
		}

		bson::r::Document document1;
		const bson::r::Document& readDocument = mongoResponse->document;
		if(readDocument.Get("cursor", document1))
		{
			return XCode::DataBaseFindDataEmpty;
		}
		std::list<bson::r::Document> results;
		if(document1.Get("firstBatch", results))
		{
			return XCode::DataBaseFindDataEmpty;
		}
		std::unique_ptr<json::w::Value> jsonArray = response.AddArray("list");
		for (bson::r::Document& document: results)
		{
			std::string json;
			if(document.WriterToJson(&json))
			{
				jsonArray->PushObject(json);
			}
		}
		return XCode::Ok;
	}
}