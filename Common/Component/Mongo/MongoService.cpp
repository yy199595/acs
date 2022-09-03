//
// Created by mac on 2022/5/19.
//

#include"MongoService.h"
#include"MongoRpcComponent.h"


namespace Sentry
{
    MongoService::MongoService()
        : mMongoComponent(nullptr)
    {

    }
	bool MongoService::OnStartService(ServiceMethodRegister& methodRegister)
    {
        this->mMongoComponent = this->GetComponent<MongoRpcComponent>();
        methodRegister.Bind("Query", &MongoService::Query);
        methodRegister.Bind("Insert", &MongoService::Insert);
        methodRegister.Bind("Delete", &MongoService::Delete);
        methodRegister.Bind("Update", &MongoService::Update);
        methodRegister.Bind("SetIndex", &MongoService::SetIndex);
        methodRegister.Bind("RunCommand", &MongoService::RunCommand);
        return this->mMongoComponent != nullptr;
    }


    XCode MongoService::RunCommand(const db::mongo::command::request &request, db::mongo::command::response &response)
    {
        std::shared_ptr<MongoQueryRequest> mongoRequest
                = std::make_shared<MongoQueryRequest>();
        if (!mongoRequest->document.FromByJson(request.json()))
        {
            return XCode::CallArgsError;
        }
        std::shared_ptr<TcpMongoClient> mongoClient = this->mMongoComponent->GetClient();
        std::shared_ptr<MongoQueryResponse> mongoResponse = this->mMongoComponent->Run(mongoClient, mongoRequest);
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
		std::shared_ptr<MongoQueryRequest> mongoRequest
			= std::make_shared<MongoQueryRequest>();

		const std::string& tab = request.tab();
		mongoRequest->document.Add("insert", tab);

		if (!document.FromByJson(request.json()))
		{
			return XCode::CallArgsError;
		}
		Bson::Writer::Array documentArray;
		documentArray.Add(document);
		mongoRequest->document.Add("documents", documentArray);
		std::shared_ptr<TcpMongoClient> mongoClient = this->mMongoComponent->GetClient(request.flag());
		std::shared_ptr<MongoQueryResponse> response = this->mMongoComponent->Run(mongoClient, mongoRequest);
		if (response == nullptr || response->GetDocumentSize() <= 0)
		{
#ifdef __DEBUG__
			LOG_ERROR("insert [" << tab << "] failure json =" << request.json());
#endif
			return XCode::Failure;
		}
		double res = 0;
		return response->Get().Get("n", res) && res > 0 ? XCode::Successful : XCode::Failure;
	}

    XCode MongoService::Delete(const db::mongo::remove &request)
    {
        Bson::Writer::Document document;
        if(!document.FromByJson(request.json()))
		{
			return XCode::CallArgsError;
		}
        Bson::Writer::Document delDocument;
        delDocument.Add("q", document);
        delDocument.Add("limit", request.limit());

        Bson::Writer::Array documentArray(delDocument);
        std::shared_ptr<MongoQueryRequest> mongoRequest
                = std::make_shared<MongoQueryRequest>();

        mongoRequest->document.Add("deletes", documentArray);
        mongoRequest->document.Add("delete", request.tab());
        std::shared_ptr<TcpMongoClient> mongoClient = this->mMongoComponent->GetClient(request.flag());
        std::shared_ptr<MongoQueryResponse> response = this->mMongoComponent->Run(mongoClient, mongoRequest);
        if(response == nullptr || response->GetDocumentSize() <= 0)
        {
#ifdef __DEBUG__
            LOG_ERROR("delete [" << request.tab() << "] failure json =" << request.json());
#endif
            return XCode::Failure;
        }
        int count = 0;
        return response->Get().Get("n", count) && count > 0 ? XCode::Successful : XCode::Failure;
    }

    // $gt:大于   $lt:小于  $gte:大于或等于  $lte:小于或等于 $ne:不等于
    XCode MongoService::Update(const db::mongo::update &request)
    {
        Bson::Writer::Document dataDocument;
        if(!dataDocument.FromByJson(request.update()))
        {
                return XCode::CallArgsError;
        }
        Bson::Writer::Document selectorDocument;
        if(!selectorDocument.FromByJson(request.select()))
        {
                return XCode::CallArgsError;
        }
        Bson::Writer::Document updateDocument;
        updateDocument.Add(request.tag().c_str(), dataDocument);
        std::shared_ptr<MongoQueryRequest> mongoRequest(new MongoQueryRequest);

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
        std::shared_ptr<MongoQueryResponse> response = this->mMongoComponent->Run(mongoClient, mongoRequest);
        if(response == nullptr || response->GetDocumentSize() == 0)
        {
            return XCode::Failure;
        }
        int count = 0;
		Bson::Reader::Document & result = response->Get();
        return result.Get("n", count) && count > 0 ? XCode::Successful : XCode::Failure;
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
        std::shared_ptr<MongoQueryRequest> mongoRequest
                = std::make_shared<MongoQueryRequest>();
        if (!mongoRequest->document.FromByJson(request.json()))
		{
			return XCode::CallArgsError;
		}
        mongoRequest->numberToReturn = request.limit();
        const Mongo::Config &config = this->mMongoComponent->GetConfig();
        std::shared_ptr<TcpMongoClient> mongoClient = this->mMongoComponent->GetClient();
        mongoRequest->collectionName = fmt::format("{0}.{1}", config.mDb, request.tab());
        std::shared_ptr<MongoQueryResponse> queryResponse = this->mMongoComponent->Run(mongoClient, mongoRequest);

        if (queryResponse == nullptr || queryResponse->GetDocumentSize() <= 0)
        {
            return XCode::Failure;
        }
        for (size_t index = 0; index < queryResponse->GetDocumentSize(); index++)
        {
            std::string *json = response.add_jsons();
            queryResponse->Get().WriterToJson(*json);
        }
        return XCode::Successful;
    }

}