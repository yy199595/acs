//
// Created by mac on 2022/5/19.
//

#include"MongoService.h"
#include"MongoRpcComponent.h"
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
        LOG_CHECK_RET_FALSE(this->mMongoComponent = this->GetComponent<MongoRpcComponent>());
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
		Bson::Writer::Document document;
		std::shared_ptr<CommandRequest> mongoRequest
			= std::make_shared<CommandRequest>();

		const std::string& tab = request.tab();
		mongoRequest->document.Add("insert", tab);

        std::string id;
		if (!document.FromByJson(request.json(), id))
		{
			return XCode::CallArgsError;
		}
		Bson::Writer::Array documentArray;
		documentArray.Add(document);
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
                this->mSyncRedisComponent->Set(id, tab, request.json());
            }
            return XCode::Successful;
        }
        return XCode::Failure;
	}

    XCode MongoService::Delete(const db::mongo::remove &request)
    {
        std::string id;
        Bson::Writer::Document document;
        if(!document.FromByJson(request.json(), id))
		{
			return XCode::CallArgsError;
		}
        Bson::Writer::Document delDocument;
        delDocument.Add("q", document);
        delDocument.Add("limit", request.limit());

        Bson::Writer::Array documentArray(delDocument);
        std::shared_ptr<CommandRequest> mongoRequest
                = std::make_shared<CommandRequest>();

        mongoRequest->document.Add("deletes", documentArray);
        mongoRequest->document.Add("delete", request.tab());
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
        if(response->Get().Get("n", count) && count > 0)
        {
            if(!id.empty() && this->mSyncRedisComponent != nullptr)
            {
                this->mSyncRedisComponent->Del(id, request.tab());
            }
            return XCode::Successful;
        }
        return XCode::Failure;
    }

    // $gt:大于   $lt:小于  $gte:大于或等于  $lte:小于或等于 $ne:不等于
    XCode MongoService::Update(const db::mongo::update &request)
    {
        Bson::Writer::Document dataDocument;
        if(!dataDocument.FromByJson(request.update()))
        {
                return XCode::CallArgsError;
        }
        std::string id;
        Bson::Writer::Document selectorDocument;
        if(!selectorDocument.FromByJson(request.select(), id))
        {
                return XCode::CallArgsError;
        }
        Bson::Writer::Document updateDocument;
        updateDocument.Add(request.tag().c_str(), dataDocument);
        std::shared_ptr<CommandRequest> mongoRequest(new CommandRequest);

        Bson::Writer::Document updateInfo;
        const std::string & tab = request.tab();
        updateInfo.Add("multi", true);
        updateInfo.Add("upsert", false);
        updateInfo.Add("u", updateDocument);
        updateInfo.Add("q", selectorDocument);

        Bson::Writer::Array updates(updateInfo);
        mongoRequest->document.Add("update", tab);
        mongoRequest->document.Add("updates", updates);
        std::shared_ptr<TcpMongoClient> mongoClient = this->mMongoComponent->GetClient(request.flag());
        std::shared_ptr<CommandResponse> response = this->mMongoComponent->Run(mongoClient, mongoRequest);
        if(response == nullptr || response->GetDocumentSize() == 0)
        {
            return XCode::Failure;
        }
        int count = 0;
		Bson::Reader::Document & result = response->Get();
        if(result.Get("n", count) && count > 0)
        {
            if(!id.empty() && this->mSyncRedisComponent != nullptr)
            {
                this->mSyncRedisComponent->Del(id, request.tab());
            }
            return XCode::Successful;
        }
        return XCode::Failure;
    }

    XCode MongoService::SetIndex(const db::mongo::index &request)
    {
        const std::string & tab = request.tab();
        const std::string & name = request.name();
        if(!this->mMongoComponent->SetIndex(tab, name))
        {
            return XCode::Failure;
        }
        return XCode::Successful;
    }

    XCode MongoService::Query(const db::mongo::query::request &request, db::mongo::query::response &response)
    {
        std::shared_ptr<CommandRequest> mongoRequest
                = std::make_shared<CommandRequest>();
        if (!mongoRequest->document.FromByJson(request.json()))
		{
			return XCode::CallArgsError;
		}
        mongoRequest->numberToReturn = request.limit();
        const Mongo::Config &config = this->mMongoComponent->GetConfig();
        std::shared_ptr<TcpMongoClient> mongoClient = this->mMongoComponent->GetClient();
        mongoRequest->collectionName = fmt::format("{0}.{1}", config.mDb, request.tab());
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
                if(document->Get("_id", stringId))
                {
                    this->mSyncRedisComponent->Set(stringId, tab, *json);
                }
                else if (document->Get("_id", numberId))
                {
                    stringId = std::to_string(numberId);
                    this->mSyncRedisComponent->Set(stringId, tab, *json);
                }
            }
        }
        return XCode::Successful;
    }

}