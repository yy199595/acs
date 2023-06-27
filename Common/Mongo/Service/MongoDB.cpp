//
// Created by mac on 2022/5/19.
//

#include"MongoDB.h"
#include"Entity/Actor/App.h"
#include"Mongo/Config/MongoConfig.h"
#include"Mongo/Component/MongoDBComponent.h"
#include"Mongo/Client/MongoFactory.h"
namespace Tendo
{
    MongoDB::MongoDB()
    {
        this->mIndex = 0;
        this->mMongoComponent = nullptr;
    }

    bool MongoDB::Awake()
    {
    	std::string path;
    	this->mApp->Config()->GetPath("db", path);
    	this->mApp->AddComponent<MongoDBComponent>();
		LOG_CHECK_RET_FALSE(this->mConfig.LoadConfig(path));
    	return true;
    }

	bool MongoDB::OnInit()
    {
        BIND_COMMON_RPC_METHOD(MongoDB::Save);
        BIND_COMMON_RPC_METHOD(MongoDB::Find);
        BIND_COMMON_RPC_METHOD(MongoDB::Insert);
        BIND_COMMON_RPC_METHOD(MongoDB::Delete);
        BIND_COMMON_RPC_METHOD(MongoDB::Update);
        BIND_COMMON_RPC_METHOD(MongoDB::SetIndex);
        BIND_COMMON_RPC_METHOD(MongoDB::Command);
		BIND_COMMON_RPC_METHOD(MongoDB::FindOne);
		this->mMongoComponent = this->GetComponent<MongoDBComponent>();
		for(int index = 0; index < this->mConfig.MaxCount; index++)
		{
			int id = this->mMongoComponent->MakeMongoClient(this->mConfig);
			{
				this->mClients.emplace_back(id);
			}
		}
        return true;
    }

    int MongoDB::GetClientHandle(int flag)
    {
        if (flag == 0)
        {
            int id = this->mClients[this->mIndex++];
            if (this->mIndex >= this->mClients.size())
            {
                this->mIndex = 0;
            }
            return id;
        }
        int index = flag % this->mClients.size();
        return this->mClients[index];
    }

    int MongoDB::Command(const db::mongo::command::request &request, db::mongo::command::response &response)
    {
		const std::string & tab = request.tab();
		const std::string & cmd = request.cmd();
		const std::string & json = request.json();
        const std::shared_ptr<Mongo::CommandRequest> mongoRequest = Mongo::MongoFactory::Command(tab, cmd, json);
        if (mongoRequest == nullptr)
        {
            return XCode::CallArgsError;
        }
        int id = this->GetClientHandle();
        const std::shared_ptr<Mongo::CommandResponse> mongoResponse = this->mMongoComponent->Run(id, mongoRequest);
        if (mongoResponse != nullptr && mongoResponse->Document() != nullptr)
        {
			mongoResponse->Document()->WriterToJson(response.mutable_json());
            return XCode::Successful;
        }
        return XCode::Failure;
    }

    int MongoDB::Insert(const db::mongo::insert &request)
	{
		Bson::Writer::Document document;
		const std::string & tab = request.tab();
		if (!document.FromByJson(request.json()))
		{
			return XCode::CallArgsError;
		}
		const std::shared_ptr<Mongo::CommandRequest> mongoRequest = Mongo::MongoFactory::Insert(tab, document);
		if(mongoRequest == nullptr)
		{
			return XCode::CallArgsError;
		}
		int res = 0;
		int handle = this->GetClientHandle();
		const std::shared_ptr<Mongo::CommandResponse> mongoResponse =
							this->mMongoComponent->Run(handle, mongoRequest);
		LOG_ERROR_RETURN_CODE(mongoResponse && mongoResponse->Document(), XCode::Failure);

		Bson::Reader::Document * document1 = mongoResponse->Document();
		return document1->Get("n", res) && res > 0 ? XCode::Successful : XCode::Failure;
	}

    int MongoDB::Delete(const db::mongo::remove &request)
    {
        Bson::Writer::Document document;
        if(!document.FromByJson(request.json()))
		{
			return XCode::CallArgsError;
		}
		int limit = request.limit();
		const std::string & tab = request.tab();
        const std::shared_ptr<Mongo::CommandRequest> mongoRequest = Mongo::MongoFactory::Delete(tab, document, limit);
		if(mongoRequest == nullptr)
		{
			return XCode::CallArgsError;
		}
		int count = 0;
		int handle = this->GetClientHandle();
		std::shared_ptr<Mongo::CommandResponse> mongoResponse
					= this->mMongoComponent->Run(handle, mongoRequest);
		LOG_ERROR_RETURN_CODE(mongoResponse && mongoResponse->Document(), XCode::Failure);
		Bson::Reader::Document * document1 = mongoResponse->Document();
        return (document1->Get("n", count) && count >= limit) ? XCode::Successful : XCode::Failure;
    }

    // $gt:大于   $lt:小于  $gte:大于或等于  $lte:小于或等于 $ne:不等于
    int MongoDB::Update(const db::mongo::update &request)
    {
        return this->UpdateData(request, false);
    }

    int MongoDB::Save(const db::mongo::update& request)
    {
        return this->UpdateData(request, true);
    }

    int MongoDB::SetIndex(const db::mongo::index &request)
    {
		std::vector<std::string> keys;
		for(const std::string & index : request.keys())
		{
			keys.emplace_back(index);
		}
        const std::string & tab = request.tab();
		std::shared_ptr<Mongo::CommandRequest> commandRequest = Mongo::MongoFactory::CreateIndex(tab, keys);
		if(commandRequest == nullptr)
		{
			return XCode::CallArgsError;
		}
		int id = this->mClients.front();
		std::shared_ptr<Mongo::CommandResponse> mongoResponse
					= this->mMongoComponent->Run(id, commandRequest);
		LOG_ERROR_RETURN_CODE(mongoResponse && mongoResponse->Document(), XCode::Failure);
		return XCode::Successful;
    }

	int MongoDB::FindOne(const db::mongo::find_one::request& request, db::mongo::find_one::response& response)
	{
		const std::string & tab = request.tab();
		const std::string & json = request.where();
		std::shared_ptr<Mongo::CommandRequest> mongoRequest = Mongo::MongoFactory::Query(tab, json);
		if(mongoRequest == nullptr)
		{
			return XCode::CallArgsError;
		}
		for(const std::string & field : request.fields())
		{
			mongoRequest->fields.Add(field.c_str(), 1);
		}
		int handle = this->GetClientHandle();
		std::shared_ptr<Mongo::CommandResponse> mongoResponse =
				this->mMongoComponent->Run(handle, mongoRequest);

		LOG_ERROR_RETURN_CODE(mongoResponse && mongoResponse->Document(), XCode::Failure);

		Bson::Reader::Document * readDocument = mongoResponse->Document();

		std::shared_ptr<Bson::Reader::Document> document1;
		LOG_ERROR_RETURN_CODE(readDocument->Get("cursor", document1), XCode::Failure);

		std::vector<std::shared_ptr<Bson::Reader::Document>> results;
		LOG_ERROR_RETURN_CODE(document1->Get("firstBatch", results), XCode::Failure);
		for(std::shared_ptr<Bson::Reader::Document> & json : results)
		{
			json->WriterToJson(response.mutable_json());
			return XCode::Successful;
		}
		return XCode::Successful;
	}

    int MongoDB::Find(const db::mongo::find::request &request, db::mongo::find::response &response)
    {
		const int limit = request.limit();
		const std::string & tab = request.tab();
		const std::string & json = request.json();
         std::shared_ptr<Mongo::CommandRequest> mongoRequest = Mongo::MongoFactory::Query(tab, json, limit);
		if(mongoRequest == nullptr)
		{
			return XCode::CallArgsError;
		}
		for(const std::string & field : request.fields())
		{
			mongoRequest->fields.Add(field.c_str(), 1);
		}
        int handle = this->GetClientHandle();
        std::shared_ptr<Mongo::CommandResponse> mongoResponse =
					this->mMongoComponent->Run(handle, mongoRequest);

		LOG_ERROR_RETURN_CODE(mongoResponse && mongoResponse->Document(), XCode::Failure);

		Bson::Reader::Document * readDocument = mongoResponse->Document();

		std::shared_ptr<Bson::Reader::Document> document1;
		LOG_ERROR_RETURN_CODE(readDocument->Get("cursor", document1), XCode::Failure);

		std::vector<std::string> results;
		LOG_ERROR_RETURN_CODE(document1->Get("firstBatch", results), XCode::Failure);
		for(const std::string & json : results)
		{
			response.add_jsons(json);
		}
        return XCode::Successful;
    }

    int MongoDB::UpdateData(const db::mongo::update &request, bool upsert)
    {
        Bson::Writer::Document dataDocument;
        if (!dataDocument.FromByJson(request.update()))
        {
            return XCode::CallArgsError;
        }
        Bson::Writer::Document selectorDocument;
        if (!selectorDocument.FromByJson(request.select()))
        {
            return XCode::CallArgsError;
        }
		const std::string & table = request.tab();
		const char * tag = request.tag().empty() ? "$set" : request.tag().c_str();
        const std::shared_ptr<Mongo::CommandRequest> mongoRequest = Mongo::MongoFactory::Update(table, selectorDocument, dataDocument, tag, upsert);
		if(mongoRequest == nullptr)
		{
			return XCode::CallArgsError;
		}
        int handle = this->GetClientHandle();
        const std::shared_ptr<Mongo::CommandResponse> mongoResponse = this->mMongoComponent->Run(handle, mongoRequest);
		LOG_ERROR_RETURN_CODE(mongoResponse && mongoResponse->Document(), XCode::Failure);

        int count = 0;
		Bson::Reader::Document * result = mongoResponse->Document();
        return (result->Get("n", count) && count > 0) ? XCode::Successful : XCode::Failure;
    }

}