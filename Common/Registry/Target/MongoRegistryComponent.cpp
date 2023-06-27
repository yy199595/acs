#include"MongoRegistryComponent.h"
#include"Entity/Actor/App.h"
#include"XCode/XCode.h"
#include"s2s/db.pb.h"
#include"Mongo/Client/BsonDocument.h"
#include"Mongo/Client/MongoFactory.h"
#include"Mongo/Component/MongoDBComponent.h"
namespace Tendo
{
	MongoRegistryComponent::MongoRegistryComponent()
	{
		this->mIndex = 0;
		this->mMongo = nullptr;
	}

	bool MongoRegistryComponent::Awake()
	{
		std::string path;
		this->mApp->Config()->GetPath("db", path);
		this->mApp->AddComponent<MongoDBComponent>();
		LOG_CHECK_RET_FALSE(this->mConfig.LoadConfig(path));
		return true;
	}

	bool MongoRegistryComponent::LateAwake()
	{
		this->mMongo = this->mApp->GetComponent<MongoDBComponent>();
		this->mIndex = this->mMongo->MakeMongoClient(this->mConfig, false);
		return true;
	}
	
	int MongoRegistryComponent::Del(const std::string& name, long long id)
	{
		Bson::Writer::Document document;
		document.Add("_id", id);
		const std::string tab = fmt::format("registry.{1}", name);
		std::shared_ptr<Mongo::CommandRequest> mongoRequest = Mongo::MongoFactory::Delete(tab, document);
		std::shared_ptr<Mongo::CommandResponse> response = this->mMongo->Run(this->mIndex, mongoRequest);
		return response == nullptr ? XCode::Failure : XCode::Successful;
	}

	int MongoRegistryComponent::Add(const std::string& name, long long id, const std::string& json)
	{
		Bson::Writer::Document update;
		if(!update.FromByJson(json))
		{
			return XCode::CallArgsError;
		}
		Bson::Writer::Document select;
		select.Add("_id", id);
		const std::string tab = fmt::format("registry.{0}", name);
		std::shared_ptr<Mongo::CommandRequest> mongoRequest =
				Mongo::MongoFactory::Update(tab, select, update, "$set", true);
		std::shared_ptr<Mongo::CommandResponse> mongoResponse = this->mMongo->Run(this->mIndex, mongoRequest);

		LOG_ERROR_RETURN_CODE(mongoResponse && mongoResponse->Document(), XCode::Failure);

		int count = 0;
		const Bson::Reader::Document * result = mongoResponse->Document();
		return (result->Get("n", count) && count > 0) ? XCode::Successful : XCode::Failure;
	}

	int MongoRegistryComponent::Query(const std::string& table, registry::query::response& response)
	{
		const std::string tab = fmt::format("registry.{0}", table);
		std::shared_ptr<Mongo::CommandRequest> mongoRequest = Mongo::MongoFactory::Query(tab, "{}", 0);
		std::shared_ptr<Mongo::CommandResponse> mongoResponse = this->mMongo->Run(this->mIndex, mongoRequest);

		LOG_ERROR_RETURN_CODE(mongoResponse && mongoResponse->Document(), XCode::Failure);
		return this->Encode(mongoResponse->Document(), response);
	}

	int MongoRegistryComponent::Query(const std::string& table, long long id, registry::query::response& response)
	{
		Json::Writer jsonWriter;
		jsonWriter.Add("_id").Add(id);
		const std::string json = jsonWriter.JsonString();
		const std::string tab = fmt::format("registry.{0}", table);
		std::shared_ptr<Mongo::CommandRequest> mongoRequest = Mongo::MongoFactory::Query(tab, json, 1);
		std::shared_ptr<Mongo::CommandResponse> mongoResponse = this->mMongo->Run(this->mIndex, mongoRequest);

		LOG_ERROR_RETURN_CODE(mongoResponse && mongoResponse->Document(), XCode::Failure);
		return this->Encode(mongoResponse->Document(), response);
	}

	int MongoRegistryComponent::Query(const string& table, const string& name, registry::query::response& response)
	{
		Json::Writer jsonWriter;
		jsonWriter.Add("name").Add(name);
		const std::string json = jsonWriter.JsonString();
		const std::string tab = fmt::format("registry.{0}", table);
		std::shared_ptr<Mongo::CommandRequest> mongoRequest = Mongo::MongoFactory::Query(tab, json, 0);
		std::shared_ptr<Mongo::CommandResponse> mongoResponse = this->mMongo->Run(this->mIndex, mongoRequest);
		LOG_ERROR_RETURN_CODE(mongoResponse && mongoResponse->Document(), XCode::Failure);

		return this->Encode(mongoResponse->Document(), response);
	}

	int MongoRegistryComponent::Encode(Bson::Reader::Document* readDocument, registry::query::response& response)
	{
		std::shared_ptr<Bson::Reader::Document> document1;
		LOG_ERROR_RETURN_CODE(readDocument->Get("cursor", document1), XCode::Failure);

		std::vector<std::shared_ptr<Bson::Reader::Document>> results;
		LOG_ERROR_RETURN_CODE(document1->Get("firstBatch", results), XCode::Failure);
		for(std::shared_ptr<Bson::Reader::Document> & json : results)
		{
			json->WriterToJson(response.add_actors());
			return XCode::Successful;
		}
		return XCode::Successful;
	}

}