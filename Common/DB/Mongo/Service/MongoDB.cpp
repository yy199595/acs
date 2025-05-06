//
// Created by mac on 2022/5/19.
//

#include"MongoDB.h"
#include"Entity/Actor/App.h"
#include"Util/File/DirectoryHelper.h"
#include"Mongo/Client/MongoFactory.h"
#include"Mongo/Component/MongoDBComponent.h"


namespace acs
{
	MongoDB::MongoDB()
	{
		this->mMongo = nullptr;
	}

	bool MongoDB::Awake()
	{
		this->mApp->AddComponent<MongoDBComponent>();
		return true;
	}

	bool MongoDB::OnInit()
	{
		BIND_RPC_METHOD(MongoDB::Run);
		BIND_RPC_METHOD(MongoDB::Inc);
		BIND_RPC_METHOD(MongoDB::Find);
		BIND_RPC_METHOD(MongoDB::Count);
		BIND_RPC_METHOD(MongoDB::Insert);
		BIND_RPC_METHOD(MongoDB::Delete);
		BIND_RPC_METHOD(MongoDB::Update);
		BIND_RPC_METHOD(MongoDB::Facet);
		BIND_RPC_METHOD(MongoDB::GetMore);
		BIND_RPC_METHOD(MongoDB::Updates);
		BIND_RPC_METHOD(MongoDB::SetIndex);
		BIND_RPC_METHOD(MongoDB::FindOne);
		BIND_RPC_METHOD(MongoDB::FindPage);
		BIND_RPC_METHOD(MongoDB::Distinct);
		BIND_RPC_METHOD(MongoDB::Databases);
		BIND_RPC_METHOD(MongoDB::Aggregate);
		BIND_RPC_METHOD(MongoDB::Collections);
		BIND_RPC_METHOD(MongoDB::FindAndModify);
		LOG_CHECK_RET_FALSE(this->mMongo = this->GetComponent<MongoDBComponent>())
		return true;
	}

	int MongoDB::Databases(com::array::string& response)
	{
		std::unique_ptr<mongo::Request> request1 = std::make_unique<mongo::Request>();
		{
			bson::Writer::Document doc1;
			doc1.Add("nameOnly", true);
			request1->dataBase = "admin";
			request1->document.Add("listDatabases", doc1);
		}
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(std::move(request1));
		if (mongoResponse == nullptr)
		{
			return XCode::Failure;
		}

		std::vector<std::unique_ptr<bson::Reader::Document>> results;
		const bson::Reader::Document& readDocument = mongoResponse->Document();
		LOG_ERROR_RETURN_CODE(readDocument.Get("databases", results), XCode::FindMongoDocumentFail);
		for (std::unique_ptr<bson::Reader::Document>& document: results)
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
		if (request.str().empty())
		{
			return XCode::CallArgsError;
		}
		std::unique_ptr<mongo::Request> message = std::make_unique<mongo::Request>();
		{
			bson::Writer::Document doc1;
			doc1.Add("nameOnly", true);
			message->dataBase = request.str();
			message->document.Add("listCollections", doc1);
		}
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(std::move(message));
		if (mongoResponse == nullptr)
		{
			return XCode::Failure;
		}
		const bson::Reader::Document& readDocument = mongoResponse->Document();

		std::unique_ptr<bson::Reader::Document> document1;
		LOG_ERROR_RETURN_CODE(readDocument.Get("cursor", document1), XCode::FindMongoDocumentFail);

		std::vector<std::unique_ptr<bson::Reader::Document>> results;
		LOG_ERROR_RETURN_CODE(document1->Get("firstBatch", results), XCode::FindMongoDocumentFail);
		for (std::unique_ptr<bson::Reader::Document>& document: results)
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
		if (!mongo::MongoFactory::New("counter", "findAndModify", mongoRequest))
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
		response.set_value(value);
		return XCode::Ok;
	}

	int MongoDB::FindAndModify(const db::mongo::find_modify::request& request,
			db::mongo::find_modify::response& response)
	{
		const std::string& tab = request.tab();
		std::unique_ptr<mongo::Request> mongoRequest;
		if (!mongo::MongoFactory::New(tab, "findAndModify", mongoRequest))
		{
			return XCode::Failure;
		}
		bson::Writer::Document query;
		bson::Writer::Document update;
		if (!query.FromByJson(request.filter()))
		{
			return XCode::CallArgsError;
		}
		if (!update.FromByJson(request.update()))
		{
			return XCode::CallArgsError;
		}
		mongoRequest->document.Add("query", query);
		mongoRequest->document.Add("update", update);
		if (request.fields_size() > 0)
		{
			bson::Writer::Document mode;
			for (const std::string& field: request.fields())
			{
				mode.Add(field.c_str(), 1);
			}
			mongoRequest->document.Add("fields", mode);
		}
		bson::Writer::Document options;
		options.Add("returnDocument", "after");
		mongoRequest->document.Add("options", options);
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(std::move(mongoRequest));
		if (mongoResponse == nullptr)
		{
			return XCode::Failure;
		}
		std::unique_ptr<bson::Reader::Document> valDocument;
		if (!mongoResponse->Document().Get("value", valDocument))
		{
			return XCode::NotFoundData;
		}
		valDocument->WriterToJson(response.mutable_document());
		return XCode::Ok;
	}

	int MongoDB::Run(const db::mongo::command::request& request, db::mongo::command::response& response)
	{
		json::r::Document jsonDocument;
		const std::string& tab = request.tab();
		const std::string& cmd = request.cmd();
		const std::string& document = request.document();
		if (!document.empty() && !jsonDocument.Decode(document))
		{
			return XCode::ParseJsonFailure;
		}
		std::unique_ptr<mongo::Request> mongoRequest = mongo::MongoFactory::Command(tab, cmd, jsonDocument);
		if (mongoRequest == nullptr)
		{
			return XCode::CallArgsError;
		}
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(std::move(mongoRequest));
		if (mongoResponse == nullptr)
		{
			return XCode::Failure;
		}
		mongoResponse->Encode(response.mutable_document());
		return XCode::Ok;
	}

	int MongoDB::Insert(const db::mongo::insert& request, db::mongo::response& response)
	{
		const std::string& tab = request.tab();
		if (request.documents_size() == 0)
		{
			return XCode::CallArgsError;
		}
		std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
		{
			bson::Writer::Array documents;
			for (int index = 0; index < request.documents_size(); index++)
			{
				bson::Writer::Document document;
				if (!document.FromByJson(request.documents(index)))
				{
					return XCode::ParseJsonFailure;
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
		if (mongoResponse->Document().Get("n", count))
		{
			response.set_count(count);
		}
		return count > 0 ? XCode::Ok : XCode::InsertMongoDocumentFail;
	}

	int MongoDB::Delete(const db::mongo::remove& request, db::mongo::response& response)
	{
		bson::Writer::Document document;
		if (!document.FromByJson(request.filter()))
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
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(std::move(mongoRequest));
		if (mongoResponse == nullptr)
		{
			return XCode::Failure;
		}
		if (mongoResponse->Document().Get("n", count))
		{
			response.set_count(count);
		}
		return count >= limit ? XCode::Ok : XCode::Failure;
	}

	// $gt:大于   $lt:小于  $gte:大于或等于  $lte:小于或等于 $ne:不等于
	int MongoDB::Update(const db::mongo::update& request, db::mongo::response& response)
	{
		return this->UpdateData(request, request.upsert(), request.multi(), response);
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
		const std::string& tab = request.tab();

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
				const char * cmd = "$set";
				if(!info.cmd().empty())
				{
					cmd = info.cmd().c_str();
				}
				updateDocument.Add(cmd, updater);
			}
			bson::Writer::Document updateInfo;
			updateInfo.Add("multi", false); //默认更新一个文档
			updateInfo.Add("upsert", false); //true不存在插入
			updateInfo.Add("u", updateDocument);
			updateInfo.Add("q", filter);

			updates.Add(updateInfo);
		}
		mongoRequest->document.Add("updates", updates);
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(std::move(mongoRequest));
		if (mongoResponse == nullptr)
		{
			return XCode::Failure;
		}

		int count = 0;
		if (mongoResponse->Document().Get("nModified", count))
		{
			response.set_count(count);
		}
		return count > 0 ? XCode::Ok : XCode::UpdateMongoDocumentFail;
	}

	int MongoDB::GetMore(const db::mongo::find::more& request, db::mongo::find::response& response)
	{
		long long cursor = request.cursor();
		std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
		{
			mongoRequest->document.Add("getMore", cursor);
			mongoRequest->GetCollection("collection", request.tab());
			if (request.batchsize() > 0)
			{
				mongoRequest->document.Add("batchSize", request.batchsize());
			}

			mongoRequest->cmd = "getMore";
			std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(std::move(mongoRequest));
			if (mongoResponse == nullptr)
			{
				return XCode::Failure;
			}
			response.set_cursor(mongoResponse->GetCursor());
			for (const std::string& document: mongoResponse->GetResults())
			{
				response.add_documents(document);
			}
		}
		return XCode::Ok;
	}

	int MongoDB::SetIndex(const db::mongo::index& request)
	{
		int sort = request.sort();
		bool unique = request.unique();
		const std::string& tab = request.tab();
		const std::string& key = request.key();
		LOG_ERROR_CHECK_ARGS(sort == 1 || sort == -1)
		std::unique_ptr<mongo::Request> commandRequest =
				mongo::MongoFactory::CreateIndex(tab, key, sort, unique);
		if (commandRequest == nullptr)
		{
			return XCode::CallArgsError;
		}
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(std::move(commandRequest));
		if (mongoResponse == nullptr)
		{
			return XCode::Failure;
		}
		return XCode::Ok;
	}

	int MongoDB::Distinct(const db::mongo::find::distinct& request, db::mongo::find_one::response& response)
	{
		const std::string& tab = request.tab();
		const std::string& key = request.key();
		if (tab.empty() || key.empty())
		{
			return XCode::CallArgsError;
		}
		const std::string& filter = request.filter();
		std::unique_ptr<mongo::Request> mongoRequest;
		mongo::MongoFactory::New(tab, "distinct", mongoRequest);
		{
			mongoRequest->document.Add("key", request.key());
			if (!filter.empty())
			{
				bson::Writer::Document filterDocument;
				if (!filterDocument.FromByJson(filter))
				{
					return XCode::ParseJsonFailure;
				}
				mongoRequest->document.Add("query", filterDocument);
			}
		}
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(std::move(mongoRequest));
		if (mongoResponse == nullptr || !mongoResponse->Document().IsOk())
		{
			return XCode::Failure;
		}
		response.set_document(mongoResponse->ToString());
		return XCode::Ok;
	}

	int MongoDB::FindOne(const db::mongo::find_one::request& request, db::mongo::find_one::response& response)
	{
		bson::Writer::Document document2;
		if (!document2.FromByJson(request.filter()))
		{
			return XCode::CallArgsError;
		}
		const std::string& tab = request.tab();
		std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
		{
			mongoRequest->GetCollection("find", tab).Filter(document2).Limit(1);
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
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(std::move(mongoRequest));
		if (mongoResponse == nullptr || mongoResponse->GetResults().empty())
		{
			return XCode::NotFoundData;
		}
		response.set_document(mongoResponse->GetResults().front());
		return XCode::Ok;
	}

	int MongoDB::FindPage(const db::mongo::find::page& request, db::mongo::find::response& response)
	{
		int limit = request.count();
		if (limit <= 0)
		{
			limit = 10;
		}
		const std::string& tab = request.tab();
		const std::string& filter = request.filter();
		const int skip = (request.page() - 1) * limit;
		std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
		{
			bson::Writer::Document document;
			mongoRequest->GetCollection("find", tab);
			if (!filter.empty() && document.FromByJson(filter))
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

		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(std::move(mongoRequest));
		if (mongoResponse == nullptr)
		{
			return XCode::Failure;
		}
		for (const std::string& json: mongoResponse->GetResults())
		{
			response.add_documents(json);
		}
		return XCode::Ok;
	}

	int MongoDB::Find(const db::mongo::find::request& request, db::mongo::find::response& response)
	{
		json::r::Document jsonDocument;
		const std::string& tab = request.tab();
		const std::string& filter = request.filter();
		if (!filter.empty() && !jsonDocument.Decode(filter))
		{
			return XCode::ParseJsonFailure;
		}

		std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
		{
			bson::Writer::Document filterDocument;
			mongoRequest->GetCollection("find", tab);
			if (filterDocument.FromByJson(jsonDocument))
			{
				mongoRequest->Filter(filterDocument);
			}
			if (request.limit() > 0)
			{
				mongoRequest->Limit(request.limit());
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
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(std::move(mongoRequest));
		if (mongoResponse == nullptr)
		{
			return XCode::Failure;
		}
		for (const std::string& json: mongoResponse->GetResults())
		{
			response.add_documents(json);
		}
		response.set_cursor(mongoResponse->GetCursor());
		return XCode::Ok;
	}

	int MongoDB::Count(const db::mongo::count::request& request, db::mongo::count::response& response)
	{
		const std::string& tab = request.tab();
		const std::string& filter = request.filter();
		std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
		{
			bson::Writer::Document document;
			mongoRequest->GetCollection("count", tab);
			if (!filter.empty() && document.FromByJson(filter))
			{
				mongoRequest->Query(document);
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
		response.set_count(count);
		return XCode::Ok;
	}

	int MongoDB::UpdateData(const db::mongo::update& request, bool upsert, bool multi, db::mongo::response& response)
	{
		bson::Writer::Document dataDocument;
		bson::Writer::Document selectorDocument;
		const std::string& filter = request.document().filter();
		if (!filter.empty() && !selectorDocument.FromByJson(filter))
		{
			return XCode::CallArgsError;
		}
		if (!dataDocument.FromByJson(request.document().document()))
		{
			return XCode::CallArgsError;
		}
		const char * cmd = "$set";
		if(!request.document().cmd().empty())
		{
			cmd = request.document().cmd().c_str();
		}
		const std::string& table = request.tab();
		std::unique_ptr<mongo::Request> mongoRequest = mongo::MongoFactory::Update(table, selectorDocument,
				dataDocument, cmd, upsert, multi);
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
		if (mongoResponse->Document().Get("nModified", count))
		{
			response.set_count(count);
		}
		return count > 0 ? XCode::Ok : XCode::UpdateMongoDocumentFail;
	}

	int MongoDB::Facet(const db::mongo::aggregate::facet& request, db::mongo::find::response& response)
	{
		const std::string& tab = request.tab();
		if (tab.empty() || request.match_size() <= 0)
		{
			return XCode::CallArgsError;
		}
		bson::Writer::Document groupDocument;
		{
			bson::Writer::Document group;
			if (request._id().empty())
			{
				group.Add("_id");
			}
			else
			{
				group.Add("_id", fmt::format("${}", request._id()));
			}
			if (!group.FromByJson(request.group()))
			{
				return XCode::Failure;
			}
			groupDocument.Add("$group", group);
		}
		bson::Writer::Array pipeline;
		std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
		{
			mongoRequest->GetCollection("aggregate", tab);
			{
				bson::Writer::Document document1;
				bson::Writer::Document facet;

				for (int index = 0; index < request.match_size(); index++)
				{
					bson::Writer::Array facet1;
					bson::Writer::Document match;
					if (!match.FromByJson(request.match(index)))
					{
						return XCode::CallArgsError;
					}
					std::string json = match.ToString();
					bson::Writer::Document matchDocument;
					matchDocument.Add("$match", match);
					facet1.Add(matchDocument);
					facet1.Add(groupDocument);
					//string key = fmt::format("{}", index);
					facet.Add(std::to_string(index).c_str(), facet1);
				}
				bson::Writer::Document facet2;
				facet2.Add("$facet", facet);
				pipeline.Add(facet2);
			}

			bson::Writer::Document cursor;
			if (request.batchsize() > 0)
			{
				cursor.Add("batchSize", request.batchsize());
			}
			mongoRequest->document.Add("allowDiskUse", true);
			mongoRequest->document.Add("cursor", cursor);
			mongoRequest->document.Add("pipeline", pipeline);
		}
		std::string req = mongoRequest->ToString();
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(std::move(mongoRequest));
		if (mongoResponse == nullptr)
		{
			return XCode::NotFoundData;
		}
		std::string res = mongoResponse->ToString();
		response.set_cursor(mongoResponse->GetCursor());
		if(mongoResponse->IsEmpty())
		{
			return XCode::NotFoundData;
		}
		json::r::Document document1;
		std::vector<std::unique_ptr<json::r::Value>> jsonValues;
		const std::string & json = mongoResponse->GetResults().front();
		if(!document1.Decode(json))
		{
			return XCode::ParseJsonFailure;
		}
		document1.GetValues(jsonValues);
		std::vector<std::unique_ptr<json::r::Value>> jsonValues1;
		for(std::unique_ptr<json::r::Value> & jsonValue : jsonValues)
		{
			if(jsonValue->IsArray() || jsonValue->IsObject())
			{
				jsonValues1.clear();
				jsonValue->GetValues(jsonValues1);
				for(std::unique_ptr<json::r::Value> & jsonValue1 : jsonValues1)
				{
					response.add_documents(jsonValue1->ToString());
				}
			}
			else
			{
				response.add_documents(jsonValue->ToString());
			}
		}
		return XCode::Ok;
	}

	int MongoDB::Aggregate(const db::mongo::aggregate::request& request, db::mongo::find::response& response)
	{
		const std::string& tab = request.tab();
		const std::string& cmd = request.cmd();
		if (tab.empty() || cmd.empty())
		{
			return XCode::CallArgsError;
		}
		std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
		{
			bson::Writer::Array pipeline;
			mongoRequest->GetCollection("aggregate", tab);
			{
				bson::Writer::Document document1;
				bson::Writer::Document firstDocument;
				if (!request.filter().empty())
				{
					bson::Writer::Document filter;
					if (!filter.FromByJson(request.filter()))
					{
						return XCode::ParseJsonFailure;
					}
					bson::Writer::Document match;
					match.Add("$match", filter);
					pipeline.Add(match);
				}
				if (!request.by().empty())
				{
					document1.Add("_id", fmt::format("${}", request.by()));
				}
				else
				{
					document1.Add("_id");
				}
				bson::Writer::Document document2;
				document2.Add(cmd.c_str(), fmt::format("${}", request.field()));
				document1.Add("value", document2);

				firstDocument.Add("$group", document1);
				pipeline.Add(firstDocument);
			}
			bson::Writer::Document cursor;
			if (request.batchsize() > 0)
			{
				cursor.Add("batchSize", request.batchsize());
			}
			mongoRequest->document.Add("cursor", cursor);
			mongoRequest->document.Add("pipeline", pipeline);
			mongoRequest->document.Add("allowDiskUse", true);
		}
		std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(std::move(mongoRequest));
		if (mongoResponse == nullptr)
		{
			return XCode::NotFoundData;
		}
		response.set_cursor(mongoResponse->GetCursor());
		for (const std::string& document: mongoResponse->GetResults())
		{
			response.add_documents(document);
		}
		return XCode::Ok;
	}
}
