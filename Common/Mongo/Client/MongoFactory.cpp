//
// Created by leyi on 2023/6/6.
//

#include "MongoFactory.h"

namespace Mongo
{
	bool MongoFactory::New(const std::string& table, const char * command, std::shared_ptr<CommandRequest>& mongoRequest)
	{
		size_t pos = table.find('.');
		if(pos == std::string::npos)
		{
			return false;
		}
		mongoRequest = std::make_shared<CommandRequest>();
		{
			const std::string tab = table.substr(pos + 1);
			mongoRequest->dataBase = table.substr(0, pos);

			mongoRequest->document.Add(command, tab);
		}
		return true;
	}

	std::shared_ptr<CommandRequest> MongoFactory::Insert(const std::string& table, Bson::Writer::Document& document)
	{
		std::shared_ptr<CommandRequest> mongoRequest;
		if(MongoFactory::New(table, "insert", mongoRequest))
		{
			Bson::Writer::Array documents(document);
			mongoRequest->document.Add("documents", documents);
		}
		return mongoRequest;
	}

	std::shared_ptr<CommandRequest> MongoFactory::Delete(const std::string& table, Bson::Writer::Document& document)
	{
		std::shared_ptr<CommandRequest> mongoRequest;
		if(MongoFactory::New(table, "delete", mongoRequest))
		{
			Bson::Writer::Document delDocument;
			{
				delDocument.Add("limit", 1);
				delDocument.Add("q", document);
			}
			Bson::Writer::Array documents(delDocument);
			mongoRequest->document.Add("deletes", documents);
		}
		return mongoRequest;
	}

	std::shared_ptr<CommandRequest> MongoFactory::Update(const std::string& table,
			Bson::Writer::Document& select, Bson::Writer::Document& update,const char* tag, bool upsert)
	{
		std::shared_ptr<CommandRequest> mongoRequest;
		if(MongoFactory::New(table, "update", mongoRequest))
		{
			Bson::Writer::Document updateDocument;
			{
				updateDocument.Add(tag, update);
			}
			Bson::Writer::Document updateInfo;
			updateInfo.Add("multi", false); //默认更新一个文档
			updateInfo.Add("upsert", upsert); //true不存在插入
			updateInfo.Add("u", updateDocument);
			updateInfo.Add("q", select);

			Bson::Writer::Array updates(updateInfo);
			mongoRequest->document.Add("updates", updates);
		}
		return mongoRequest;
	}

	std::shared_ptr<CommandRequest> MongoFactory::Query(const std::string& tab, const std::string & json, int limit)
	{
		std::shared_ptr<CommandRequest> mongoRequest = std::make_shared<CommandRequest>();
		{
			mongoRequest->collectionName = tab;
			mongoRequest->numberToReturn = limit;
			if(!mongoRequest->document.FromByJson(json))
			{
				return nullptr;
			}
		}
		return mongoRequest;
	}
}