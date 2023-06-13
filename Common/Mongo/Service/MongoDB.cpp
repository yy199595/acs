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
        BIND_COMMON_RPC_METHOD(MongoDB::Query);
        BIND_COMMON_RPC_METHOD(MongoDB::Insert);
        BIND_COMMON_RPC_METHOD(MongoDB::Delete);
        BIND_COMMON_RPC_METHOD(MongoDB::Update);
        BIND_COMMON_RPC_METHOD(MongoDB::SetIndex);
        BIND_COMMON_RPC_METHOD(MongoDB::RunCommand);
        this->mMongoComponent = this->GetComponent<MongoDBComponent>();
        return true;
    }

	void MongoDB::OnStart()
	{
		for(int index = 0; index < this->mConfig.MaxCount; index++)
		{
			int id = this->mMongoComponent->MakeMongoClient(this->mConfig);
			while(!this->mMongoComponent->Ping(id))
			{
				LOG_ERROR("try ping mongo server");
			}
		}
	}

    void MongoDB::OnStop()
    {
        
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

    int MongoDB::RunCommand(const db::mongo::command::request &request, db::mongo::command::response &response)
    {
        const std::shared_ptr<Mongo::CommandRequest> mongoRequest = std::make_shared<Mongo::CommandRequest>();
        if (!mongoRequest->document.FromByJson(request.json()))
        {
            return XCode::CallArgsError;
        }
        int id = this->GetClientHandle();    
        const std::shared_ptr<Mongo::CommandResponse> mongoResponse = this->mMongoComponent->Run(id, mongoRequest);
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
		int handle = this->GetClientHandle(request.flag());
		const std::shared_ptr<Mongo::CommandResponse> mongoResponse = this->mMongoComponent->Run(handle, mongoRequest);
		if (mongoResponse == nullptr || mongoResponse->GetDocumentSize() <= 0)
		{
#ifdef __DEBUG__
			LOG_ERROR("insert [" << tab << "] failure json =" << request.json());
#endif
			return XCode::Failure;
		}
        int res = 0;
        Bson::Reader::Document & result = mongoResponse->Get();
        return result.Get("n", res) && res > 0 ? XCode::Successful : XCode::Failure;
	}

    int MongoDB::Delete(const db::mongo::remove &request)
    {
        Bson::Writer::Document document;
        if(!document.FromByJson(request.json()))
		{
			return XCode::CallArgsError;
		}
		const std::string & tab = request.tab();
        const std::shared_ptr<Mongo::CommandRequest> mongoRequest = Mongo::MongoFactory::Delete(tab, document);
		if(mongoRequest == nullptr)
		{
			return XCode::CallArgsError;
		}
        int handle = this->GetClientHandle(request.flag());
        const std::shared_ptr<Mongo::CommandResponse> response = this->mMongoComponent->Run(handle, mongoRequest);
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
        const size_t pos = request.tab().find('.');
        if (pos == std::string::npos)
        {
            CONSOLE_LOG_ERROR("[" << request.tab() << "] parse error xxx.xxx");
            return XCode::CallArgsError;
        }
        const std::string & tab = request.tab();
        const std::string & name = request.name();
        return this->mMongoComponent->SetIndex(1, tab, name) ? XCode::Successful : XCode::Failure;
    }

    int MongoDB::Query(const db::mongo::query::request &request, db::mongo::query::response &response)
    {
        const size_t pos = request.tab().find('.');
        if(pos == std::string::npos)
        {
            return XCode::CallArgsError;
        }
		const int limit = request.limit();
		const std::string & tab = request.tab();
		const std::string & json = request.json();
         std::shared_ptr<Mongo::CommandRequest> mongoRequest = Mongo::MongoFactory::Query(tab, json, limit);
		if(mongoRequest == nullptr)
		{
			return XCode::CallArgsError;
		}
        int handle = this->GetClientHandle();
        std::shared_ptr<Mongo::CommandResponse> queryResponse = this->mMongoComponent->Run(handle, mongoRequest);

        if (queryResponse == nullptr || queryResponse->GetDocumentSize() <= 0)
        {
            return XCode::Failure;
        }

        for(Bson::Reader::Document * document : *queryResponse)
        {
            std::string * json = response.add_jsons();
            document->WriterToJson(*json);
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
        int handle = this->GetClientHandle(request.flag());
        const std::shared_ptr<Mongo::CommandResponse> response = this->mMongoComponent->Run(handle, mongoRequest);
        if (response == nullptr || response->GetDocumentSize() == 0)
        {
            return XCode::Failure;
        }
        int count = 0;
        std::string json;
        response->Get().WriterToJson(json);
        const Bson::Reader::Document& result = response->Get();
        return (result.Get("n", count) && count > 0) ? XCode::Successful : XCode::Failure;
    }

}