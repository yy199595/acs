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
        this->mCounterId = "CounterKey";
        this->mCounterTable = "Common_Counter";
    }
	bool MongoService::OnStartService(ServiceMethodRegister& methodRegister)
    {
        this->mMongoComponent = this->GetComponent<MongoRpcComponent>();
        methodRegister.Bind("Query", &MongoService::Query);
        methodRegister.Bind("Insert", &MongoService::Insert);
        methodRegister.Bind("Delete", &MongoService::Delete);
        methodRegister.Bind("Update", &MongoService::Update);
        methodRegister.Bind("SetIndex", &MongoService::SetIndex);
        methodRegister.Bind("AddCounter", &MongoService::AddCounter);
        methodRegister.Bind("RunCommand", &MongoService::RunCommand);
        return this->mMongoComponent != nullptr;
    }

    bool MongoService::OnStart()
    {
        Json::Writer select;
        select << "_id" << this->mCounterId;
        this->mMongoComponent->InsertOnce(this->mCounterTable, select.JsonString());
        return this->RefreshCounter();
    }

    bool MongoService::RefreshCounter()
    {
        Json::Writer select;
        select << "_id" << this->mCounterId;
        std::shared_ptr<MongoQueryResponse> response =
                this->mMongoComponent->Query(this->mCounterTable, select.JsonString());
        if(response == nullptr)
        {
            return false;
        }
        if(response->GetDocumentSize() > 0)
        {
            std::set<std::string> keys;
            Bson::Read::Object & bsonObject = response->Get(0);
            if(bsonObject.GetKeys(keys))
            {
                for(const std::string & key : keys)
                {
                    long long value = 0;
                    if(bsonObject.Get(key.c_str(),value))
                    {
                        this->mCounters[key] = value;
                    }
                }
            }
        }
        return true;
    }

    XCode MongoService::AddCounter(const s2s::mongo::counter::request & request, s2s::mongo::counter::response &response)
    {
        Json::Writer json1;
        Json::Writer json2;
        json1 << "_id" << this->mCounterId;
        json2 << request.key() << request.value();
        if(this->mMongoComponent->Update(this->mCounterTable, json2.JsonString(), json1.JsonString(), "$inc"))
        {
            auto iter = this->mCounters.find(request.key());
            if(iter == this->mCounters.end())
            {
                this->RefreshCounter();
            }
            iter = this->mCounters.find(request.key());
            if(iter != this->mCounters.end())
            {
                response.set_counter(iter->second);
            }
        }
        return XCode::Successful;
    }

    XCode MongoService::RunCommand(const s2s::mongo::command::request &request, s2s::mongo::command::response &response)
    {
        std::shared_ptr<MongoQueryRequest> mongoRequest(new MongoQueryRequest());

        const std::string & json = request.json();
        if(!mongoRequest->document.FromByJson(json))
        {
            return XCode::CallArgsError;
        }
        std::shared_ptr<TcpMongoClient> client = this->mMongoComponent->GetClient();
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