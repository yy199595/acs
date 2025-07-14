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
		BIND_RPC_METHOD(MongoWriteProxy::Save)
		BIND_RPC_METHOD(MongoWriteProxy::Update)
		BIND_RPC_METHOD(MongoWriteProxy::Insert)
		BIND_RPC_METHOD(MongoWriteProxy::Delete)
		BIND_RPC_METHOD(MongoWriteProxy::Updates)
		BIND_RPC_METHOD(MongoWriteProxy::Deletes)
		BIND_RPC_METHOD(MongoWriteProxy::FindModify)
		LOG_CHECK_RET_FALSE(this->mMongo = this->GetComponent<MongoDBComponent>())
		return true;
	}

	int MongoWriteProxy::Insert(const json::r::Document& request, json::w::Document& response)
	{
		std::string tab;
		json::r::Value jsonArray;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("list", jsonArray) && jsonArray.IsArray())
		std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
		{
			size_t index = 0;
			json::r::Value jsonObject;
			mongoRequest->GetCollection("insert", tab);
			bson::w::Document documents(false);
			while(jsonArray.Get(index, jsonObject))
			{
				index++;
				if(!documents.Push(jsonObject))
				{
					return XCode::CallArgsError;
				}
			}
			mongoRequest->Insert(documents);
		}
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(mongoRequest);
		if (mongoResponse == nullptr)
		{
			return XCode::Failure;
		}
		int count = 0;
		if (!mongoResponse->document.Get("n", count))
		{
			return XCode::DataBaseInsertFailure;
		}
		response.Add("count", count);
		return  XCode::Ok;
	}

	int MongoWriteProxy::Inc(const json::r::Document& request, json::w::Document& response)
	{
		int value = 0;
		std::string tab;
		if(!request.Get("value", value))
		{
			value = 1;
		}
		std::string field;
		bool upsert = true;
		json::r::Value filter;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab));
		LOG_ERROR_CHECK_ARGS(request.Get("upsert", upsert));
		LOG_ERROR_CHECK_ARGS(request.Get("field", field));
		LOG_ERROR_CHECK_ARGS(request.Get("filter", filter));

		std::unique_ptr<mongo::Request> mongoRequest;
		if (!mongo::MongoFactory::New(tab, "findAndModify", mongoRequest))
		{
			return XCode::Failure;
		}
		mongoRequest->document.Add("query", filter);
		bson::w::Document update;
		{
			update.AddObject("$inc", field.c_str(), value);
			mongoRequest->document.Add("update", update);
		}
		mongoRequest->document.Add("upsert", upsert);
		mongoRequest->document.Add("new", true);
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(mongoRequest);
		if (mongoResponse == nullptr)
		{
			return XCode::Failure;
		}
		bson::r::Document valDocument;
		if (!mongoResponse->document.Get("value", valDocument))
		{
			return XCode::DataBaseUpdateFailure;
		}
		int targetValue = 0;
		if (!valDocument.Get(field.c_str(), targetValue))
		{
			return XCode::DataBaseFindDataEmpty;
		}
		response.Add("value", targetValue);
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
		json::r::Value filter, document;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("filter", filter) && filter.IsObject())
		LOG_ERROR_CHECK_ARGS(request.Get("document", document) && document.IsObject())
		{
			bson::w::Document dataDocument;
			bson::w::Document selectorDocument;
			if(!selectorDocument.FromByJson(filter))
			{
				return XCode::CallArgsError;
			}
			if(!dataDocument.FromByJson(document))
			{
				return XCode::CallArgsError;
			}
			std::unique_ptr<mongo::Request> mongoRequest = mongo::MongoFactory::Update(tab,
					selectorDocument, dataDocument, cmd.c_str(), upsert, multi);
			if (mongoRequest == nullptr)
			{
				return XCode::CallArgsError;
			}
			std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(mongoRequest);
			if (mongoResponse == nullptr)
			{
				return XCode::Failure;
			}
			int count = 0;
			if (!mongoResponse->document.Get("nModified", count))
			{
				return XCode::DataBaseUpdateFailure;
			}
			response.Add("count", count);
		}
		return XCode::Ok;
	}

	int MongoWriteProxy::Save(const json::r::Document& request, json::w::Document& response)
	{
		std::string tab;
		bool upsert = false;
		json::r::Value filter;
		json::r::Value document;
		request.Get("upsert", tab);
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("filter", filter))
		LOG_ERROR_CHECK_ARGS(request.Get("document", document))
		std::unique_ptr<mongo::Request> mongoRequest;
		mongo::MongoFactory::New(tab, "update", mongoRequest);
		{
			bson::w::Document updateInfo;
			updateInfo.Add("multi", false); //更新多个文档
			updateInfo.Add("upsert", upsert); //true不存在插入
			updateInfo.Add("u", document);
			updateInfo.Add("q", filter);
			bson::w::Document updates(updateInfo);
			mongoRequest->document.Add("updates", updates);
		}
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(mongoRequest);
		if (mongoResponse == nullptr)
		{
			return XCode::Failure;
		}
		int count = 0;
		if (!mongoResponse->document.Get("nModified", count))
		{
			return XCode::DataBaseUpdateFailure;
		}
		response.Add("count", count);
		return XCode::Ok;
	}

	int MongoWriteProxy::Delete(const json::r::Document& request, json::w::Document& response)
	{
		int limit = 1;
		std::string tab;
		json::r::Value filter;
		request.Get("limit", limit);
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("filter", filter) && filter.IsObject())
		bson::w::Document filterDocument;
		if(!filterDocument.FromByJson(filter))
		{
			return XCode::CallArgsError;
		}
		std::unique_ptr<mongo::Request> mongoRequest = mongo::MongoFactory::Delete(tab, filterDocument, limit);
		if (mongoRequest == nullptr)
		{
			return XCode::CallArgsError;
		}
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(mongoRequest);
		if (mongoResponse == nullptr)
		{
			return XCode::Failure;
		}
		int count = 0;
		if (!mongoResponse->document.Get("n", count))
		{
			return XCode::DataBaseDeleteFailure;
		}
		response.Add("count", count);
		return XCode::Ok;
	}

	int MongoWriteProxy::Deletes(const json::r::Document& request, json::w::Document& response)
	{
		std::string tab;
		json::r::Value jsonArray;
		std::unique_ptr<mongo::Request> mongoRequest;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("list", jsonArray) && jsonArray.IsArray())
		{
			if(!mongo::MongoFactory::New(tab, "delete", mongoRequest))
			{
				return XCode::CallArgsError;
			}
			size_t index = 0;
			json::r::Value jsonObject;
			bson::w::Document documents(false);
			while(jsonArray.Get(index, jsonObject))
			{
				index++;
				bson::w::Document delDocument;
				{
					delDocument.Add("limit", 1);
					if(!delDocument.Add("q", jsonObject))
					{
						return XCode::CallArgsError;
					}
				}
				documents.Push(delDocument);
			}
			mongoRequest->document.Add("deletes", documents);
		}
		std::string req = mongoRequest->ToString();
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(mongoRequest);
		if (mongoResponse == nullptr)
		{
			return XCode::Failure;
		}
		int count = 0;
		if (!mongoResponse->document.Get("n", count))
		{
			return XCode::DataBaseDeleteFailure;
		}
		response.Add("count", count);
		return XCode::Ok;
	}

	int MongoWriteProxy::Updates(const json::r::Document& request, json::w::Document& response)
	{
		std::string tab;
		json::r::Value jsonArray;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("list", jsonArray) && jsonArray.IsArray())
		{
			std::unique_ptr<mongo::Request> mongoRequest;
			if(!mongo::MongoFactory::New(tab, "update", mongoRequest))
			{
				return XCode::CallArgsError;
			}
			size_t index = 0;
			json::r::Value jsonObject;
			bson::w::Document updates(false);
			while(jsonArray.Get(index, jsonObject))
			{
				index++;
				std::string cmd = "$set";
				jsonObject.Get("cmd", cmd);
				json::r::Value filter, document;
				if(jsonObject.Get("filter", filter) && jsonObject.Get("document", document))
				{
					bson::w::Document updateInfo;
					{
						updateInfo.Add("multi", false); //默认更新一个文档
						updateInfo.Add("upsert", false); //true不存在插入
						LOG_ERROR_CHECK_ARGS(updateInfo.Add("u", document))
						LOG_ERROR_CHECK_ARGS(updateInfo.Add("q", filter))
					}
					updates.Push(updateInfo);
				}
			}
			mongoRequest->document.Add("updates", updates);
			std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(mongoRequest);
			if (mongoResponse == nullptr)
			{
				return XCode::Failure;
			}

			int count = 0;
			if (!mongoResponse->document.Get("nModified", count))
			{
				return XCode::DataBaseUpdateFailure;
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
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(commandRequest);
		return mongoResponse != nullptr ? XCode::Ok : XCode::Failure;
	}

	int MongoWriteProxy::FindModify(const json::r::Document& request, json::w::Document& response)
	{
		std::string tab;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		std::unique_ptr<mongo::Request> mongoRequest;
		if (!mongo::MongoFactory::New(tab, "findAndModify", mongoRequest))
		{
			return XCode::Failure;
		}

		std::string cmd("$set");
		json::r::Value filter;
		json::r::Value document;
		request.Get("cmd", cmd);
		LOG_ERROR_CHECK_ARGS(request.Get("filter", filter))
		LOG_ERROR_CHECK_ARGS(request.Get("document", document))
		{
			mongoRequest->document.Add("query", filter);
			mongoRequest->document.AddObject("update", cmd.c_str(), document);
		}
		std::vector<std::string> fields;
		if(request.Get("fields", fields) && !fields.empty())
		{
			bson::w::Document fieldObject;
			for (const std::string& field: fields)
			{
				fieldObject.Add(field.c_str(), 1);
			}
			mongoRequest->document.Add("fields", fieldObject);
		}
		mongoRequest->document.Add("new", true);
		mongoRequest->document.AddObject("options", "returnDocument", "after");
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(mongoRequest);
		if (mongoResponse == nullptr)
		{
			return XCode::Failure;
		}
		bson::r::Document valDocument;
		if (!mongoResponse->document.Get("value", valDocument))
		{
			return XCode::DataBaseFindDataEmpty;
		}
		valDocument.WriterToJson(response);
		return XCode::Ok;
	}
}