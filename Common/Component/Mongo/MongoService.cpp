//
// Created by mac on 2022/5/19.
//

#include"MongoService.h"
#include"MongoDBComponent.h"
#include"DataSyncComponent.h"

namespace Sentry
{
    MongoService::MongoService()
        : mMongoComponent(nullptr)
    {
        this->mMongoComponent = nullptr;
        this->mSyncRedisComponent = nullptr;
    }
	bool MongoService::OnStartService(ServiceMethodRegister& methodRegister)
    {
        methodRegister.Bind("Query", &MongoService::Query);
        methodRegister.Bind("Insert", &MongoService::Insert);
        methodRegister.Bind("Delete", &MongoService::Delete);
        methodRegister.Bind("Update", &MongoService::Update);
        methodRegister.Bind("SetIndex", &MongoService::SetIndex);
        methodRegister.Bind("RunCommand", &MongoService::RunCommand);
        this->mSyncRedisComponent = this->GetComponent<DataSyncComponent>();
        LOG_CHECK_RET_FALSE(this->mMongoComponent = this->GetComponent<MongoDBComponent>());
        return true;
    }


    XCode MongoService::RunCommand(const db::mongo::command::request &request, db::mongo::command::response &response)
    {
        std::shared_ptr<CommandRequest> mongoRequest
                = std::make_shared<CommandRequest>();
        if (!mongoRequest->document.FromByJson(request.json()))
        {
            return XCode::CallArgsError;
        }
        std::shared_ptr<TcpMongoClient> mongoClient = this->mMongoComponent->GetClient();
        std::shared_ptr<CommandResponse> mongoResponse = this->mMongoComponent->Run(mongoClient, mongoRequest);
        if (mongoResponse != nullptr)
        {
            for (size_t index = 0; index < mongoResponse->GetDocumentSize(); index++)
            {
                std::string *json = response.add_jsons();
                mongoResponse->Get().WriterToJson(*json);
            }
            return XCode::Successful;
        }
        return XCode::Failure;
    }

    XCode MongoService::Insert(const db::mongo::insert &request)
	{
        const size_t pos = request.tab().find('.');
        if(pos == std::string::npos)
        {
            CONSOLE_LOG_ERROR("[" << request.tab() << "] parse error xxx.xxx");
            return XCode::CallArgsError;
        }

        std::string id;
        Bson::Writer::Document document;
        if (!document.FromByJson(request.json(), id))
        {
            return XCode::CallArgsError;
        }

        std::shared_ptr<CommandRequest> mongoRequest
            = std::make_shared<CommandRequest>();

        const std::string tab = request.tab().substr(pos + 1);
        mongoRequest->dataBase = request.tab().substr(0, pos);

        Bson::Writer::Array documentArray(document);
        mongoRequest->document.Add("insert", tab);
		mongoRequest->document.Add("documents", documentArray);
		std::shared_ptr<TcpMongoClient> mongoClient = this->mMongoComponent->GetClient(request.flag());
		std::shared_ptr<CommandResponse> response = this->mMongoComponent->Run(mongoClient, mongoRequest);
		if (response == nullptr || response->GetDocumentSize() <= 0)
		{
#ifdef __DEBUG__
			LOG_ERROR("insert [" << tab << "] failure json =" << request.json());
#endif
			return XCode::Failure;
		}
        double res = 0;
        Bson::Reader::Document & result = response->Get();
        if(result.Get("n", res) && res > 0)
        {
            if(this->mSyncRedisComponent != nullptr && !id.empty())
            {
                const std::string & db = mongoRequest->dataBase;
                this->mSyncRedisComponent->Set(id, db,  tab, request.json());
            }
            return XCode::Successful;
        }
        return XCode::Failure;
	}

    XCode MongoService::Delete(const db::mongo::remove &request)
    {
        const size_t pos = request.tab().find('.');
        if(pos == std::string::npos)
        {
            CONSOLE_LOG_ERROR("[" << request.tab() << "] parse error xxx.xxx");
            return XCode::CallArgsError;
        }

        std::string id;
        Bson::Writer::Document document;
        if(!document.FromByJson(request.json(), id))
		{
			return XCode::CallArgsError;
		}

        std::shared_ptr<CommandRequest> mongoRequest
            = std::make_shared<CommandRequest>();
        const std::string tab = request.tab().substr(pos + 1);
        mongoRequest->dataBase = request.tab().substr(0, pos);

        if(!id.empty() && this->mSyncRedisComponent != nullptr)
        {
            const std::string &db = mongoRequest->dataBase;
            this->mSyncRedisComponent->Del(id, db, request.tab());
        }

        Bson::Writer::Document delDocument;
        delDocument.Add("q", document);
        delDocument.Add("limit", request.limit());

        Bson::Writer::Array documentArray(delDocument);

        mongoRequest->document.Add("delete", tab);
        mongoRequest->document.Add("deletes", documentArray);
        std::shared_ptr<TcpMongoClient> mongoClient = this->mMongoComponent->GetClient(request.flag());
        std::shared_ptr<CommandResponse> response = this->mMongoComponent->Run(mongoClient, mongoRequest);
        if(response == nullptr || response->GetDocumentSize() <= 0)
        {
#ifdef __DEBUG__
            LOG_ERROR("delete [" << request.tab() << "] failure json =" << request.json());
#endif
            return XCode::Failure;
        }
        int count = 0;
        return (response->Get().Get("n", count) && count > 0) ? XCode::Successful : XCode::Failure;
    }

    // $gt:大于   $lt:小于  $gte:大于或等于  $lte:小于或等于 $ne:不等于
    XCode MongoService::Update(const db::mongo::update &request)
    {
        const size_t pos = request.tab().find('.');
        if (pos == std::string::npos)
        {
            CONSOLE_LOG_ERROR("[" << request.tab() << "] parse error xxx.xxx");
            return XCode::CallArgsError;
        }

        Bson::Writer::Document dataDocument;
        if (!dataDocument.FromByJson(request.update()))
        {
            return XCode::CallArgsError;
        }
        std::string id;
        Bson::Writer::Document selectorDocument;
        if (!selectorDocument.FromByJson(request.select(), id))
        {
            return XCode::CallArgsError;
        }
        std::shared_ptr<CommandRequest> mongoRequest(new CommandRequest);

        const std::string tab = request.tab().substr(pos + 1);
        mongoRequest->dataBase = request.tab().substr(0, pos);

        if (!id.empty() && this->mSyncRedisComponent != nullptr)
        {
            const std::string & db = mongoRequest->dataBase;
            this->mSyncRedisComponent->Del(id, db, request.tab());
        }

        Bson::Writer::Document updateDocument;
        updateDocument.Add(request.tag().c_str(), dataDocument);

        Bson::Writer::Document updateInfo;
        updateInfo.Add("multi", true);
        updateInfo.Add("upsert", false);
        updateInfo.Add("u", updateDocument);
        updateInfo.Add("q", selectorDocument);


        Bson::Writer::Array updates(updateInfo);
        mongoRequest->document.Add("update", tab);
        mongoRequest->document.Add("updates", updates);
        std::shared_ptr<TcpMongoClient> mongoClient = this->mMongoComponent->GetClient(request.flag());
        std::shared_ptr<CommandResponse> response = this->mMongoComponent->Run(mongoClient, mongoRequest);
        if (response == nullptr || response->GetDocumentSize() == 0)
        {
            return XCode::Failure;
        }
        int count = 0;
        Bson::Reader::Document &result = response->Get();
        return (result.Get("n", count) && count > 0) ? XCode::Successful : XCode::Failure;
    }

    XCode MongoService::SetIndex(const db::mongo::index &request)
    {
        const size_t pos = request.tab().find('.');
        if (pos == std::string::npos)
        {
            CONSOLE_LOG_ERROR("[" << request.tab() << "] parse error xxx.xxx");
            return XCode::CallArgsError;
        }
        const std::string & tab = request.tab();
        const std::string & name = request.name();
        return this->mMongoComponent->SetIndex(tab, name) ? XCode::Successful : XCode::Failure;
    }

    XCode MongoService::Query(const db::mongo::query::request &request, db::mongo::query::response &response)
    {
        const size_t pos = request.tab().find('.');
        if(pos == std::string::npos)
        {
            return XCode::CallArgsError;
        }
        std::shared_ptr<CommandRequest> mongoRequest
                = std::make_shared<CommandRequest>();
        if (!mongoRequest->document.FromByJson(request.json()))
		{
			return XCode::CallArgsError;
		}
        mongoRequest->collectionName = request.tab();
        mongoRequest->numberToReturn = request.limit();
        std::shared_ptr<TcpMongoClient> mongoClient = this->mMongoComponent->GetClient();
        std::shared_ptr<CommandResponse> queryResponse = this->mMongoComponent->Run(mongoClient, mongoRequest);

        if (queryResponse == nullptr || queryResponse->GetDocumentSize() <= 0)
        {
            return XCode::Failure;
        }

        for(Bson::Reader::Document * document : *queryResponse)
        {
            std::string * json = response.add_jsons();
            document->WriterToJson(*json);
            if(this->mSyncRedisComponent != nullptr)
            {
                long long numberId = 0;
                std::string stringId = "";
                const std::string & tab = request.tab();
                const std::string & db = mongoRequest->dataBase;
                if(document->Get("_id", stringId))
                {
                    this->mSyncRedisComponent->Set(stringId, db, tab, *json);
                }
                else if (document->Get("_id", numberId))
                {
                    stringId = std::to_string(numberId);
                    this->mSyncRedisComponent->Set(stringId, db, tab, *json);
                }
            }
        }
        return XCode::Successful;
    }

}