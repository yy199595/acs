#include"MongoRegistry.h"
#include"Entity/Actor/App.h"
#include"XCode/XCode.h"
#include"s2s/db.pb.h"
#include"Mongo/Client/BsonDocument.h"
#include"Mongo/Client/MongoFactory.h"
#include"Mongo/Component/MongoDBComponent.h"
namespace Tendo
{
	MongoRegistry::MongoRegistry()
	{
		this->mIndex = 0;
		this->mMongo = nullptr;
	}

	bool MongoRegistry::Awake()
	{
		std::string path;
		this->mApp->Config()->GetPath("db", path);
		this->mApp->AddComponent<MongoDBComponent>();
		LOG_CHECK_RET_FALSE(this->mConfig.LoadConfig(path));
		return true;
	}

	bool MongoRegistry::LateAwake()
	{
		this->mMongo = this->mApp->GetComponent<MongoDBComponent>();
		this->mIndex = this->mMongo->MakeMongoClient(this->mConfig);
		return true;
	}

	void MongoRegistry::Start()
	{
		while(!this->mMongo->Ping(this->mIndex))
		{
			this->mApp->GetCoroutine()->Sleep(1000);
		}
	}

	
	int MongoRegistry::Del(const std::string& name, long long id)
	{
		std::shared_ptr<Mongo::CommandRequest> mongoRequest = std::make_shared<Mongo::CommandRequest>();
		{
			mongoRequest->dataBase = "Registry";
			std::string tab = fmt::format("{0}.{1}", mongoRequest->dataBase, name);
			
			Bson::Writer::Document document;
			document.Add("_id", id);
			Bson::Writer::Document delDocument;
			delDocument.Add("q", document);
			delDocument.Add("limit", 1);

			Bson::Writer::Array documentArray(delDocument);
			mongoRequest->document.Add("delete", name);
			mongoRequest->document.Add("deletes", documentArray);
		}
		std::shared_ptr<Mongo::CommandResponse> response = this->mMongo->Run(this->mIndex, mongoRequest);
		return 0;
	}

	int MongoRegistry::Add(const std::string& name, long long id, const std::string& json)
	{
		Json::Writer select;
		select.Add("_id").Add(id);
		const std::string func("MongoDB.Save");
		const std::string tab = fmt::format("Registry.{0}", name);
		db::mongo::update request;
		{
			request.set_tab(tab);
			request.set_update(json);
			request.set_flag((int)id);
			select.WriterStream(request.mutable_select());
		};
		return this->mApp->Call(func, request);
	}

	int MongoRegistry::Query(const std::string& name, registry::query::response& response)
	{
		return 0;
	}

	int MongoRegistry::Query(const std::string& name, long long id, registry::query::response& response)
	{
		return 0;
	}

}