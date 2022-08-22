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

        Json::Writer json(false);
        json << 1 << 2 << 3 << 4;
        std::string str = json.JsonString();

        Bson::Writer::Object add;
        add.FromByJson(str);

        int length = 0;
        const char * str1 = add.Serialize(length);

        Bson::Read::Object obj(str1);

        std::string json1;
        obj.WriterToJson(json1);

        return this->mMongoComponent != nullptr;
    }

    XCode MongoService::RunCommand(const s2s::mongo::command::request &request, s2s::mongo::command::response &response)
    {
        std::shared_ptr<MongoQueryRequest> mongoRequest(new MongoQueryRequest());

        const std::string & json = request.json();
        if(!mongoRequest->document.FromByJson(json))
        {
            return XCode::CallArgsError;
        }
        std::shared_ptr<MongoClientContext> client = this->mMongoComponent->GetClient();
        std::shared_ptr<MongoQueryResponse> mongoResponse = this->mMongoComponent->Run(client, mongoRequest);
        if(mongoResponse != nullptr)
        {
            for(size_t index = 0; index < mongoResponse->GetDocumentSize(); index++)
            {
                std::string * json = response.add_jsons();
                mongoResponse->Get().WriterToJson(*json);
            }
            return XCode::Successful;
        }
        return XCode::Failure;
    }

    XCode MongoService::Insert(const s2s::mongo::insert &request)
    {
        const int index = request.flag();
        const std::string & tab = request.tab();
        const std::string & json = request.json();
        this->mMongoComponent->SelectMongoClient(index);
        if(!this->mMongoComponent->InsertOnce(tab, json))
        {
#ifdef __DEBUG__
            LOG_ERROR("insert [" << tab << "] failure json =" << json);
#endif
            return XCode::Failure;
        }
        return XCode::Successful;
    }

    XCode MongoService::Delete(const s2s::mongo::remove &request)
    {
        int limit = request.limit();
        const int index = request.flag();
        const std::string & tab = request.tab();
        const std::string & json = request.json();
        this->mMongoComponent->SelectMongoClient(index);
        if(!this->mMongoComponent->Delete(tab, json, limit))
        {
#ifdef __DEBUG__
            LOG_ERROR("delete [" << tab << "] failure json =" << json);
#endif
            return XCode::Failure;
        }
        return XCode::Successful;
    }

    XCode MongoService::Update(const s2s::mongo::update &request)
    {
        const std::string & tab = request.tab();
        const std::string & tag = request.tag();
        const std::string & select = request.select();
        const std::string & update = request.update();
        this->mMongoComponent->SelectMongoClient(request.flag());
        if(!this->mMongoComponent->Update(tab, update, select, tag))
        {
#ifdef __DEBUG__
            LOG_ERROR("update [" << tab << "] failure select ="
                << select << " update = " << update);
#endif
            return XCode::Failure;
        }
        return XCode::Successful;
    }

    XCode MongoService::SetIndex(const s2s::mongo::index &request)
    {
        const std::string & tab = request.tab();
        const std::string & name = request.name();
        if(!this->mMongoComponent->SetIndex(tab, name))
        {
            return XCode::Failure;
        }
        return XCode::Successful;
    }

    XCode MongoService::Query(const s2s::mongo::query::request &request, s2s::mongo::query::response &response)
    {
        int limit = request.limit();
        const std::string & tab = request.tab();
        const std::string & json = request.json();
        std::shared_ptr<MongoQueryResponse> queryResponse = this->mMongoComponent->Query(tab, json, limit);
        if(queryResponse == nullptr || queryResponse->GetDocumentSize() <= 0)
        {
            return XCode::Failure;
        }
        for(size_t index = 0; index < queryResponse->GetDocumentSize(); index++)
        {
            std::string * json = response.add_jsons();
            queryResponse->Get().WriterToJson(*json);
        }
        return XCode::Successful;
    }

}