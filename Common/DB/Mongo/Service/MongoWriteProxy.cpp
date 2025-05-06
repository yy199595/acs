//
// Created by 64658 on 2025/4/23.
//

#include "MongoWriteProxy.h"
#include "Mongo/Client/MongoFactory.h"
#include "Mongo/Component/MongoDBComponent.h"
namespace acs
{
	MongoWriteProxy::MongoWriteProxy()
	{
		this->mMongo = nullptr;
	}

	bool MongoWriteProxy::OnInit()
	{
		BIND_RPC_METHOD(MongoWriteProxy::Inc)
		BIND_RPC_METHOD(MongoWriteProxy::Update)
		BIND_RPC_METHOD(MongoWriteProxy::Insert)
		BIND_RPC_METHOD(MongoWriteProxy::Delete)
		BIND_RPC_METHOD(MongoWriteProxy::Updates)
		LOG_CHECK_RET_FALSE(this->mMongo = this->GetComponent<MongoDBComponent>())
		return true;
	}

	int MongoWriteProxy::Insert(const json::r::Document& request, json::w::Document& response)
	{
		std::string tab;
		std::unique_ptr<json::r::Value> jsonArray;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("list", jsonArray) && jsonArray->IsArray())
		std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
		{
			size_t index = 0;
			bson::Writer::Array documents;
			std::unique_ptr<json::r::Value> jsonObject;
			while(jsonArray->Get(index, jsonObject))
			{
				index++;
				bson::Writer::Document document;
				if(!document.FromByJson(*jsonObject))
				{
					return XCode::CallArgsError;
				}
				documents.Add(document);
			}
			mongoRequest->GetCollection("insert", tab).Insert(documents);
		}
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(std::move(mongoRequest));
		if (mongoResponse == nullptr)
		{
			return XCode::Failure;
		}
		int count = 0;
		if (!mongoResponse->Document().Get("n", count))
		{
			return XCode::InsertMongoDocumentFail;
		}
		response.Add("count", count);
		return  XCode::Ok;
	}

	int MongoWriteProxy::Inc(const json::r::Document& request, json::w::Document& response)
	{
		std::string tab;
		std::string key;
		if(!request.Get("tab", tab))
		{
			tab = "counter";
		}
		LOG_ERROR_CHECK_ARGS(request.Get("key", key));
		std::unique_ptr<mongo::Request> mongoRequest;
		if (!mongo::MongoFactory::New("counter", "findAndModify", mongoRequest))
		{
			return XCode::Failure;
		}
		bson::Writer::Document query;
		query.Add("key", key);
		mongoRequest->document.Add("query", query);

		bson::Writer::Document update;
		bson::Writer::Document inc;
		inc.Add("value", 1);
		update.Add("$inc", inc);

		mongoRequest->document.Add("update", update);
		mongoRequest->document.Add("upsert", true);
		mongoRequest->document.Add("new", true);
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(std::move(mongoRequest));
		if (mongoResponse == nullptr)
		{
			return XCode::FindMongoDocumentFail;
		}
		std::unique_ptr<bson::Reader::Document> valDocument;
		if (!mongoResponse->Document().Get("value", valDocument))
		{
			return XCode::FindMongoDocumentFail;
		}
		int value = 0;
		if (!valDocument->Get("value", value))
		{
			return XCode::FindMongoDocumentFail;
		}
		response.Add("value", value);
		return XCode::Ok;
	}

	int MongoWriteProxy::Update(const json::r::Document& request, json::w::Document& response)
	{
		std::string tab;
		bool multi = false;
		bool upsert = false;
		std::string cmd("$set");
		request.Get("cmd", cmd);
		request.Get("multi", multi);
		request.Get("upsert", upsert);
		std::unique_ptr<json::r::Value> filter;
		std::unique_ptr<json::r::Value> document;
		std::unique_ptr<json::r::Value> jsonArray;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("filter", filter) && filter->IsObject())
		LOG_ERROR_CHECK_ARGS(request.Get("document", document) && document->IsObject())
		{
			bson::Writer::Document dataDocument;
			bson::Writer::Document selectorDocument;
			if(!selectorDocument.FromByJson(*filter))
			{
				return XCode::CallArgsError;
			}
			if(!dataDocument.FromByJson(*document))
			{
				return XCode::CallArgsError;
			}
			std::unique_ptr<mongo::Request> mongoRequest = mongo::MongoFactory::Update(tab,
					selectorDocument, dataDocument, cmd.c_str(), upsert, multi);
			if (mongoRequest == nullptr)
			{
				return XCode::CallArgsError;
			}
			std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(std::move(mongoRequest));
			if (mongoResponse == nullptr)
			{
				return XCode::Failure;
			}
			int count = 0;
			if (!mongoResponse->Document().Get("nModified", count))
			{
				return XCode::UpdateMongoDocumentFail;
			}
			response.Add("count", count);
		}
		return XCode::Ok;
	}

	int MongoWriteProxy::Delete(const json::r::Document& request, json::w::Document& response)
	{
		int limit = 0;
		std::string tab;
		std::unique_ptr<json::r::Value> filter;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("filter", filter) && filter->IsObject())
		bson::Writer::Document filterDocument;
		if(!filterDocument.FromByJson(*filter))
		{
			return XCode::CallArgsError;
		}
		std::unique_ptr<mongo::Request> mongoRequest = mongo::MongoFactory::Delete(tab, filterDocument, limit);
		if (mongoRequest == nullptr)
		{
			return XCode::CallArgsError;
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

	int MongoWriteProxy::Updates(const json::r::Document& request, json::w::Document& response)
	{
		std::string tab;
		std::unique_ptr<json::r::Value> jsonArray;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("list", jsonArray) && jsonArray->IsArray())
		{
			std::unique_ptr<mongo::Request> mongoRequest;
			if(!mongo::MongoFactory::New(tab, "update", mongoRequest))
			{
				return XCode::CallArgsError;
			}
			size_t index = 0;
			bson::Writer::Array updates;
			std::unique_ptr<json::r::Value> jsonObject;
			while(jsonArray->Get(index, jsonObject))
			{
				index++;
				std::string cmd = "$set";
				jsonObject->Get("cmd", cmd);
				std::unique_ptr<json::r::Value> filter;
				std::unique_ptr<json::r::Value> document;
				if(jsonObject->Get("filter", filter) && jsonObject->Get("document", document))
				{
					bson::Writer::Document bsonFilter;
					bson::Writer::Document bsonDocument;
					LOG_ERROR_CHECK_ARGS(bsonFilter.FromByJson(*filter));
					LOG_ERROR_CHECK_ARGS(bsonDocument.FromByJson(*document));
					bson::Writer::Document updateInfo;
					{
						updateInfo.Add("multi", false); //默认更新一个文档
						updateInfo.Add("upsert", false); //true不存在插入
						updateInfo.Add("u", bsonDocument);
						updateInfo.Add("q", bsonFilter);
					}
					updates.Add(updateInfo);
				}
			}
			mongoRequest->document.Add("updates", updates);
			std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(std::move(mongoRequest));
			if (mongoResponse == nullptr)
			{
				return XCode::Failure;
			}

			int count = 0;
			if (!mongoResponse->Document().Get("nModified", count))
			{
				return XCode::UpdateMongoDocumentFail;
			}
			response.Add("count", count);
		}
		return XCode::Ok;
	}

	int MongoWriteProxy::SetIndex(const json::r::Document& request)
	{
		int sort = 1;
		bool unique = false;
		std::string tab, key;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("key", key))
		LOG_ERROR_CHECK_ARGS(request.Get("sort", sort))
		LOG_ERROR_CHECK_ARGS(request.Get("unique", unique))
		std::unique_ptr<mongo::Request> commandRequest =
				mongo::MongoFactory::CreateIndex(tab, key, sort, unique);
		if (commandRequest == nullptr)
		{
			return XCode::CallArgsError;
		}
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(std::move(commandRequest));
		return mongoResponse != nullptr ? XCode::Ok : XCode::Failure;
	}
}