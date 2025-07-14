//
// Created by leyi on 2023/6/6.
//

#include "MongoFactory.h"
#include "Util/Tools/TimeHelper.h"
namespace mongo
{
	bool MongoFactory::New(const std::string& table, const char * command, std::unique_ptr<Request>& mongoRequest)
	{
		if(table.empty() || command == nullptr)
		{
			return false;
		}
		mongoRequest = std::make_unique<Request>();
		{
			mongoRequest->GetCollection(command, table);
		}
		return true;
	}

	std::unique_ptr<Request> MongoFactory::Count(const std::string& tab, const json::r::Document & json)
	{
		std::unique_ptr<Request> mongoRequest;
		MongoFactory::New(tab, "count", mongoRequest);
		if(json.IsObject() && json.MemberCount() > 0)
		{
			bson::w::Document filter;
			if (!filter.FromByJson(json))
			{
				return nullptr;
			}
			bson::w::Document mode;
			mode.Add("mode", "secondaryPreferred");
			mongoRequest->document.Add("$readPreference", mode);
			mongoRequest->document.Add("filter", filter);
		}
		return mongoRequest;
	}
	std::unique_ptr<Request> MongoFactory::Insert(const std::string& table, bson::w::Document& document)
	{
		std::unique_ptr<Request> mongoRequest;
		MongoFactory::New(table, "insert", mongoRequest);
		{
			mongoRequest->Insert(document);
		}
		return mongoRequest;
	}

	std::unique_ptr<Request> MongoFactory::Delete(const std::string& table, bson::w::Document& document, int limit)
	{
		std::unique_ptr<Request> mongoRequest;
		MongoFactory::New(table, "delete", mongoRequest);
		{
			bson::w::Document delDocument;
			{
				delDocument.Add("limit", limit);
				delDocument.Add("q", document);
			}
			bson::w::Document documents(delDocument);
			mongoRequest->document.Add("deletes", documents);
		}
		return mongoRequest;
	}

	std::unique_ptr<Request> MongoFactory::Update(const std::string& table,
			bson::w::Document& select, bson::w::Document& update,const char* tag, bool upsert, bool multi)
	{
		std::unique_ptr<Request> mongoRequest;
		MongoFactory::New(table, "update", mongoRequest);
		{
			bson::w::Document updateDocument;
			{
				updateDocument.Add(tag, update);
			}
			bson::w::Document updateInfo;
			updateInfo.Add("multi", multi); //默认更新一个文档
			updateInfo.Add("upsert", upsert); //true不存在插入
			updateInfo.Add("u", updateDocument);
			updateInfo.Add("q", select);

			bson::w::Document updates(updateInfo);
			mongoRequest->document.Add("updates", updates);
		}
		return mongoRequest;
	}

	std::unique_ptr<Request> MongoFactory::Query(const std::string& tab, const json::r::Document & json, int limit, int skip)
	{
		bson::w::Document filter;
		std::unique_ptr<Request> mongoRequest;
		MongoFactory::New(tab, "find", mongoRequest);
		if(json.IsObject() && json.MemberCount() > 0)
		{
			if(!filter.FromByJson(json))
			{
				return nullptr;
			}
		}
		bson::w::Document mode;
		mode.Add("mode", "secondaryPreferred");

		mongoRequest->document.Add("$readPreference", mode);
		mongoRequest->document.Add("filter", filter);
		if(skip > 0)
		{
			mongoRequest->document.Add("skip", skip);
		}
		mongoRequest->document.Add("limit", limit);
		return mongoRequest;
	}

	std::unique_ptr<Request> MongoFactory::Command(
			const std::string& table, const std::string & cmd, const json::r::Document & json)
	{
		std::unique_ptr<Request> mongoRequest;
		if(!table.empty() && !cmd.empty())
		{
			MongoFactory::New(table, cmd.c_str(), mongoRequest);
		}
		else
		{
			mongoRequest = std::make_unique<Request>();
		}
		if(json.MemberCount() > 0 && json.IsObject() && json.MemberCount() > 0)
		{
			if (!mongoRequest->document.FromByJson(json))
			{
				return nullptr;
			}
		}
		return mongoRequest;
	}

	std::unique_ptr<Request> MongoFactory::CreateIndex(
			const std::string& table, const std::string & name, int sort, bool unique)
	{
		std::unique_ptr<Request> mongoRequest;
		MongoFactory::New(table, "createIndexes", mongoRequest);

		bson::w::Document key;
		bson::w::Document document;
		key.Add(name.c_str(), sort); //1升序  -1降序

		document.Add("key", key);
		document.Add("unique", unique); //是否为唯一索引
		document.Add("name", name.c_str());

		bson::w::Document documentArray1(document);
		mongoRequest->document.Add("indexes", documentArray1);
		return mongoRequest;
	}
}