//
// Created by mac on 2022/5/19.
//

#include"MongoDB.h"
#include"Entity/Actor/App.h"
#include"Mongo/Config/MongoConfig.h"
#include"Mongo/Client/MongoFactory.h"
#include"Mongo/Component/MongoDBComponent.h"

namespace acs
{
    MongoDB::MongoDB()
    {
        this->mMongoComponent = nullptr;
    }

    bool MongoDB::Awake()
	{
		this->mApp->AddComponent<MongoDBComponent>();
		return true;
	}

	bool MongoDB::OnInit()
    {
		BIND_SERVER_RPC_METHOD(MongoDB::Inc);
		BIND_SERVER_RPC_METHOD(MongoDB::Sum);
		BIND_SERVER_RPC_METHOD(MongoDB::Save);
		BIND_SERVER_RPC_METHOD(MongoDB::Find);
		BIND_SERVER_RPC_METHOD(MongoDB::Count);
		BIND_SERVER_RPC_METHOD(MongoDB::Insert);
		BIND_SERVER_RPC_METHOD(MongoDB::Delete);
		BIND_SERVER_RPC_METHOD(MongoDB::Update);
		BIND_SERVER_RPC_METHOD(MongoDB::Updates);
		BIND_SERVER_RPC_METHOD(MongoDB::SetIndex);
		BIND_SERVER_RPC_METHOD(MongoDB::Command);
		BIND_SERVER_RPC_METHOD(MongoDB::FindOne);
		BIND_SERVER_RPC_METHOD(MongoDB::FindPage);
		BIND_SERVER_RPC_METHOD(MongoDB::Databases);
		BIND_SERVER_RPC_METHOD(MongoDB::Collections);
		BIND_SERVER_RPC_METHOD(MongoDB::FindAndModify);
		this->mMongoComponent = this->GetComponent<MongoDBComponent>();
		return this->mMongoComponent != nullptr;
    }

	int MongoDB::Databases(com::array::string& response)
	{
		std::unique_ptr<mongo::Request> request1 = std::make_unique<mongo::Request>();
		{
			bson::Writer::Document doc1;
			doc1.Add("nameOnly", true);
			request1->dataBase = "admin";
			request1->collectionName = "admin.$cmd";
			request1->document.Add("listDatabases", doc1);
		}
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongoComponent->Run(std::move(request1));
		LOG_ERROR_RETURN_CODE(mongoResponse && mongoResponse->Document(), XCode::Failure);

		std::vector<std::unique_ptr<bson::Reader::Document>> results;
		bson::Reader::Document* readDocument = mongoResponse->Document();
		LOG_ERROR_RETURN_CODE(readDocument->Get("databases", results), XCode::FindMongoDocumentFail);
		for (std::unique_ptr<bson::Reader::Document>& document : results)
		{
			std::string name;
			if (document->Get("name", name))
			{
				response.add_array(name);
			}
		}
		return XCode::Ok;
	}

	int MongoDB::Collections(const com::type_string& request, com::array::string& response)
	{
		std::unique_ptr<mongo::Request> message = std::make_unique<mongo::Request>();
		{
			bson::Writer::Document doc1;
			doc1.Add("nameOnly", true);
			if (!request.str().empty())
			{
				message->dataBase = request.str();
				message->collectionName = fmt::format("{}.$cmd", request.str());
			}
			message->document.Add("listCollections", doc1);
		}
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongoComponent->Run(std::move(message));
		LOG_ERROR_RETURN_CODE(mongoResponse && mongoResponse->Document(), XCode::Failure);

		bson::Reader::Document* readDocument = mongoResponse->Document();

		std::unique_ptr<bson::Reader::Document> document1;
		LOG_ERROR_RETURN_CODE(readDocument->Get("cursor", document1), XCode::FindMongoDocumentFail);

		std::vector<std::unique_ptr<bson::Reader::Document>> results;
		LOG_ERROR_RETURN_CODE(document1->Get("firstBatch", results), XCode::FindMongoDocumentFail);
		for (std::unique_ptr<bson::Reader::Document>& document : results)
		{
			std::string name;
			if (document->Get("name", name))
			{
				response.add_array(name);
			}
		}
		return XCode::Ok;
	}

	int MongoDB::Inc(const db::mongo::inc::request& request, db::mongo::inc::response& response)
	{
		std::unique_ptr<mongo::Request> mongoRequest;
		if(!mongo::MongoFactory::New("counter", "findAndModify", mongoRequest))
		{
			return XCode::Failure;
		}
		bson::Writer::Document query;
		query.Add("key", request.key());
		mongoRequest->document.Add("query", query);

		bson::Writer::Document update;
		bson::Writer::Document inc;
		inc.Add("value", 1);
		update.Add("$inc", inc);

		mongoRequest->document.Add("update", update);
		mongoRequest->document.Add("upsert", true);
		mongoRequest->document.Add("new", true);
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongoComponent->Run(std::move(mongoRequest));
		if (mongoResponse == nullptr || mongoResponse->Document() == nullptr)
		{
			return XCode::FindMongoDocumentFail;
		}
		std::unique_ptr<bson::Reader::Document> valDocument;
		if(!mongoResponse->Document()->Get("value", valDocument))
		{
			return XCode::FindMongoDocumentFail;
		}
		int value = 0;
		if(!valDocument->Get("value", value))
		{
			return XCode::FindMongoDocumentFail;
		}
		response.set_value(value);
		return XCode::Ok;
	}

	int MongoDB::FindAndModify(const db::mongo::find_modify::request& request,
			db::mongo::find_modify::response& response)
	{
		const std::string & tab = request.tab();
		std::unique_ptr<mongo::Request> mongoRequest;
		if(!mongo::MongoFactory::New(tab, "findAndModify", mongoRequest))
		{
			return XCode::Failure;
		}
		bson::Writer::Document query;
		bson::Writer::Document update;
		if(!query.FromByJson(request.query()))
		{
			return XCode::CallArgsError;
		}
		if(!update.FromByJson(request.update()))
		{
			return XCode::CallArgsError;
		}
		mongoRequest->document.Add("query", query);
		mongoRequest->document.Add("update", update);
		if(request.fields_size() > 0)
		{
			bson::Writer::Document mode;
			for(const std::string & field : request.fields())
			{
				mode.Add(field.c_str(), 1);
			}
			mongoRequest->document.Add("fields", mode);
		}
		bson::Writer::Document options;
		options.Add("returnDocument", "after");
		mongoRequest->document.Add("options", options);
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongoComponent->Run(std::move(mongoRequest));
		if(mongoResponse == nullptr || mongoResponse->Document() == nullptr)
		{
			return XCode::Failure;
		}
		std::unique_ptr<bson::Reader::Document> valDocument;
		if(!mongoResponse->Document()->Get("value", valDocument))
		{
			return XCode::NotFoundData;
		}
		valDocument->WriterToJson(response.mutable_json());
		return XCode::Ok;
	}

    int MongoDB::Command(const db::mongo::command::request &request, db::mongo::command::response &response)
    {
		json::r::Document jsonDocument;
		const std::string & tab = request.tab();
		const std::string & cmd = request.cmd();
		const std::string & json = request.json();
		if(!json.empty() && !jsonDocument.Decode(json))
		{
			return XCode::ParseJsonFailure;
		}
		std::unique_ptr<mongo::Request> mongoRequest = mongo::MongoFactory::Command(tab, cmd, jsonDocument);
        if (mongoRequest == nullptr)
        {
            return XCode::CallArgsError;
        }
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongoComponent->Run(std::move(mongoRequest));
        if (mongoResponse != nullptr && mongoResponse->Document() != nullptr)
        {
			mongoResponse->Document()->WriterToJson(response.mutable_json());
            return XCode::Ok;
        }
        return XCode::Failure;
    }

    int MongoDB::Insert(const db::mongo::insert &request, db::mongo::response& response)
	{
		const std::string & tab = request.tab();
		if(request.jsons_size() == 0)
		{
			return XCode::CallArgsError;
		}
		std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
		{
			bson::Writer::Array documents;
			for(int index = 0; index < request.jsons_size(); index++)
			{
				bson::Writer::Document document;
				if(!document.FromByJson(request.jsons(index)))
				{
					return XCode::ParseJsonFailure;
				}
				documents.Add(document);
			}
			mongoRequest->GetCollection("insert", tab).Insert(documents);
		}
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongoComponent->Run(std::move(mongoRequest));
		if(mongoResponse == nullptr ||  mongoResponse->Document() == nullptr)
		{
			return XCode::Failure;
		}
		int count = 0;
		if (mongoResponse->Document()->Get("n", count))
		{
			response.set_count(count);
		}
		return count > 0 ? XCode::Ok : XCode::InsertMongoDocumentFail;
	}

	int MongoDB::Delete(const db::mongo::remove& request, db::mongo::response& response)
	{
		bson::Writer::Document document;
		if (!document.FromByJson(request.json()))
		{
			return XCode::CallArgsError;
		}
		int limit = request.limit();
		const std::string& tab = request.tab();
		std::unique_ptr<mongo::Request> mongoRequest = mongo::MongoFactory::Delete(tab, document, limit);
		if (mongoRequest == nullptr)
		{
			return XCode::CallArgsError;
		}
		int count = 0;
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongoComponent->Run(std::move(mongoRequest));
		LOG_ERROR_RETURN_CODE(mongoResponse && mongoResponse->Document(), XCode::Failure);
		if (mongoResponse->Document()->Get("n", count))
		{
			response.set_count(count);
		}
		return  count >= limit ? XCode::Ok : XCode::Failure;
	}

    // $gt:大于   $lt:小于  $gte:大于或等于  $lte:小于或等于 $ne:不等于
    int MongoDB::Update(const db::mongo::update &request, db::mongo::response& response)
    {
        return this->UpdateData(request, request.upsert(), response);
    }

    int MongoDB::Save(const db::mongo::update& request, db::mongo::response& response)
    {
        return this->UpdateData(request, true, response);
    }

	int MongoDB::Updates(const db::mongo::updates& request, db::mongo::response& response)
	{
		if (request.tab().empty())
		{
			return XCode::CallArgsError;
		}
		if (request.document_size() == 0)
		{
			return XCode::CallArgsError;
		}
		const char* tab = request.tab().c_str();
		const char* tag = request.tag().empty() ? "$set" : request.tag().c_str();

		std::unique_ptr<mongo::Request> mongoRequest;
		mongo::MongoFactory::New(tab, "update", mongoRequest);

		bson::Writer::Array updates;
		for (int index = 0; index < request.document_size(); index++)
		{
			bson::Writer::Document filter;
			bson::Writer::Document updater;
			const db::mongo::updater& info = request.document(index);
			if (!filter.FromByJson(info.filter()))
			{
				return XCode::ParseJsonFailure;
			}
			if (!updater.FromByJson(info.document()))
			{
				return XCode::CallArgsError;
			}
			bson::Writer::Document updateDocument;
			{
				updateDocument.Add(tag, updater);
			}
			bson::Writer::Document updateInfo;
			updateInfo.Add("multi", false); //默认更新一个文档
			updateInfo.Add("upsert", false); //true不存在插入
			updateInfo.Add("u", updateDocument);
			updateInfo.Add("q", filter);

			updates.Add(updateInfo);
		}
		mongoRequest->document.Add("updates", updates);
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongoComponent->Run(std::move(mongoRequest));
		LOG_ERROR_RETURN_CODE(mongoResponse && mongoResponse->Document(), XCode::UpdateMongoDocumentFail);

		int count = 0;
		if (mongoResponse->Document()->Get("nModified", count))
		{
			response.set_count(count);
		}
		return count >= request.document_size() ? XCode::Ok : XCode::UpdateMongoDocumentFail;
	}

    int MongoDB::SetIndex(const db::mongo::index &request)
    {
		int sort = request.sort();
		bool unique = request.unique();
        const std::string & tab = request.tab();
		const std::string & key = request.key();
		LOG_ERROR_CHECK_ARGS(sort == 1 || sort == -1)
		std::unique_ptr<mongo::Request> commandRequest =
				mongo::MongoFactory::CreateIndex(tab, key, sort, unique);
		if(commandRequest == nullptr)
		{
			return XCode::CallArgsError;
		}
		std::unique_ptr<mongo::Response> mongoResponse= this->mMongoComponent->Run(std::move(commandRequest));
		if(mongoResponse == nullptr || mongoResponse->Document() == nullptr)
		{
			return XCode::Failure;
		}
		return XCode::Ok;
    }

	int MongoDB::FindOne(const db::mongo::find_one::request& request, db::mongo::find_one::response& response)
	{
		bson::Writer::Document document2;
		if(!document2.FromByJson(request.where()))
		{
			return XCode::CallArgsError;
		}
		const std::string & tab = request.tab();
		std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
		{
			mongoRequest->GetCollection("find", tab).Filter(document2).Limit(1);
		}
		if(request.fields_size() > 0)
		{
			bson::Writer::Document mode;
			for(const std::string & field : request.fields())
			{
				mode.Add(field.c_str(), 1);
			}
			mongoRequest->document.Add("projection", mode);
		}
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongoComponent->Run(std::move(mongoRequest));
		LOG_ERROR_RETURN_CODE(mongoResponse && mongoResponse->Document(), XCode::Failure);

		bson::Reader::Document * readDocument = mongoResponse->Document();

		std::unique_ptr<bson::Reader::Document> document1;
		LOG_ERROR_RETURN_CODE(readDocument->Get("cursor", document1), XCode::FindMongoDocumentFail);

		std::vector<std::unique_ptr<bson::Reader::Document>> results;
		LOG_ERROR_RETURN_CODE(document1->Get("firstBatch", results), XCode::FindMongoDocumentFail);
		for(std::unique_ptr<bson::Reader::Document> & document : results)
		{
			document->WriterToJson(response.mutable_json());
			return XCode::Ok;
		}
		return XCode::Ok;
	}

	int MongoDB::FindPage(const db::mongo::find_page::request& request, db::mongo::find_page::response& response)
	{
		int limit = request.count();
		if (limit <= 0)
		{
			limit = 10;
		}
		json::r::Document jsonDocument;
		const std::string& tab = request.tab();
		const std::string& json = request.where();
		int skip = (request.page() - 1) * limit;
		if (!json.empty() && !jsonDocument.Decode(json))
		{
			return XCode::ParseJsonFailure;
		}
		std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
		{
			bson::Writer::Document document;
			mongoRequest->GetCollection("find", tab);
			if (!json.empty() && document.FromByJson(json))
			{
				mongoRequest->Filter(document);
			}
			bson::Writer::Document sortDocument;
			mongoRequest->Skip(skip).Limit(limit);
			const std::string& sort = request.sort();
			if (!sort.empty() && sortDocument.FromByJson(sort))
			{
				mongoRequest->document.Add("sort", sortDocument);
			}
		}
		if (request.fields_size() > 0)
		{
			bson::Writer::Document mode;
			for (const std::string& field: request.fields())
			{
				mode.Add(field.c_str(), 1);
			}
			mongoRequest->document.Add("projection", mode);
		}

		std::unique_ptr<mongo::Response> mongoResponse = this->mMongoComponent->Run(std::move(mongoRequest));
		LOG_ERROR_RETURN_CODE(mongoResponse && mongoResponse->Document(), XCode::Failure);

		bson::Reader::Document* readDocument = mongoResponse->Document();

		std::unique_ptr<bson::Reader::Document> document1;
		LOG_ERROR_RETURN_CODE(readDocument->Get("cursor", document1), XCode::FindMongoDocumentFail);

		std::vector<std::unique_ptr<bson::Reader::Document>> results;
		LOG_ERROR_RETURN_CODE(document1->Get("firstBatch", results), XCode::FindMongoDocumentFail);
		for (std::unique_ptr<bson::Reader::Document>& document: results)
		{
			document->WriterToJson(response.add_json());
		}
		return XCode::Ok;
	}

    int MongoDB::Find(const db::mongo::find::request &request, db::mongo::find::response &response)
    {
		json::r::Document jsonDocument;
		const std::string & tab = request.tab();
		const std::string & json = request.json();
		if(!json.empty() && !jsonDocument.Decode(json))
		{
			return XCode::ParseJsonFailure;
		}

         std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
		{
			bson::Writer::Document filter;
			mongoRequest->GetCollection("find", tab);
			if(filter.FromByJson(jsonDocument))
			{
				mongoRequest->Filter(filter);
			}
			if(request.limit() > 0)
			{
				mongoRequest->Limit(request.limit());
			}
		}
		if(request.fields_size() > 0)
		{
			bson::Writer::Document mode;
			for(const std::string & field : request.fields())
			{
				mode.Add(field.c_str(), 1);
			}
			mongoRequest->document.Add("projection", mode);
		}
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongoComponent->Run(std::move(mongoRequest));
		LOG_ERROR_RETURN_CODE(mongoResponse && mongoResponse->Document(), XCode::FindMongoDocumentFail);

		bson::Reader::Document * readDocument = mongoResponse->Document();

		std::unique_ptr<bson::Reader::Document> document1;
		LOG_ERROR_RETURN_CODE(readDocument->Get("cursor", document1), XCode::FindMongoDocumentFail);

		std::vector<std::string> results;
		if(!document1->Get("firstBatch", results))
		{
			std::string error;
			document1->WriterToJson(&error);
			LOG_ERROR("query error = {}", error);
			return XCode::FindMongoDocumentFail;
		}
		for(const std::string & json1 : results)
		{
			response.add_jsons(json1);
		}
        return XCode::Ok;
    }

	int MongoDB::Count(const db::mongo::count::request& request, db::mongo::count::response& response)
	{
		const std::string& tab = request.tab();
		const std::string& where = request.where();
		std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
		{
			bson::Writer::Document document;
			mongoRequest->GetCollection("count", tab);
			if(!where.empty() && document.FromByJson(where))
			{
				mongoRequest->Query(document);
			}
		}
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongoComponent->Run(std::move(mongoRequest));
		LOG_ERROR_RETURN_CODE(mongoResponse && mongoResponse->Document(), XCode::Failure);

		int count = 0;
		if (!mongoResponse->Document()->Get("n", count))
		{
			return XCode::Failure;
		}
		response.set_count(count);
		return XCode::Ok;
	}

    int MongoDB::UpdateData(const db::mongo::update &request, bool upsert, db::mongo::response& response)
    {
        bson::Writer::Document dataDocument;
		bson::Writer::Document selectorDocument;
		const std::string & filter = request.document().filter();
		if (!filter.empty() && !selectorDocument.FromByJson(filter))
		{
			return XCode::CallArgsError;
		}
		if (!dataDocument.FromByJson(request.document().document()))
        {
            return XCode::CallArgsError;
        }

		const std::string & table = request.tab();
		const char * tag = request.tag().empty() ? "$set" : request.tag().c_str();
		std::unique_ptr<mongo::Request> mongoRequest = mongo::MongoFactory::Update(table, selectorDocument, dataDocument, tag, upsert);
		if(mongoRequest == nullptr)
		{
			return XCode::CallArgsError;
		}
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongoComponent->Run(std::move(mongoRequest));
		LOG_ERROR_RETURN_CODE(mongoResponse && mongoResponse->Document(), XCode::UpdateMongoDocumentFail);

        int count = 0;
		if (mongoResponse->Document()->Get("nModified", count))
		{
			response.set_count(count);
		}
        return count > 0 ? XCode::Ok : XCode::UpdateMongoDocumentFail;
    }

	int MongoDB::Sum(const db::mongo::sum::request & request, db::mongo::sum::response& response)
	{
		const std::string & tab = request.tab();
		std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
		{
			bson::Writer::Array pipeline;
			mongoRequest->GetCollection("aggregate", tab);
			{
				bson::Writer::Document document1;
				bson::Writer::Document firstDocument;
				if(!request.filter().empty())
				{
					bson::Writer::Document filter;
					if(!filter.FromByJson(request.filter()))
					{
						return XCode::ParseJsonFailure;
					}
					bson::Writer::Document match;
					match.Add("$match", filter);
					pipeline.Add(match);
				}
				if(!request.by().empty())
				{
					document1.Add("_id", fmt::format("${}", request.by()));
				}
				else
				{
					document1.Add("_id");
				}
				bson::Writer::Document document2;
				document2.Add("$sum", fmt::format("${}", request.field()));
				document1.Add("value", document2);

				firstDocument.Add("$group", document1);
				pipeline.Add(firstDocument);
			}
			bson::Writer::Document cursor;
			cursor.Add("batchSize", 30000);
			mongoRequest->document.Add("cursor", cursor);
			mongoRequest->document.Add("pipeline", pipeline);
		}
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongoComponent->Run(std::move(mongoRequest));
		std::unique_ptr<bson::Reader::Document> document1;
		LOG_ERROR_RETURN_CODE(mongoResponse->Document()->Get("cursor", document1), XCode::FindMongoDocumentFail);

		std::vector<std::unique_ptr<bson::Reader::Document>> results;
		LOG_ERROR_RETURN_CODE(document1->Get("firstBatch", results), XCode::FindMongoDocumentFail);
		for (std::unique_ptr<bson::Reader::Document>& document: results)
		{
			document->WriterToJson(response.add_json());
		}
		return XCode::Ok;
	}
}