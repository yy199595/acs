//
// Created by mac on 2022/5/19.
//

#include"MongoDB.h"
#include"Config/MongoConfig.h"
#include"Component/MongoDBComponent.h"
namespace Sentry
{
    MongoDB::MongoDB()
        : mMongoComponent(nullptr)
    {
        this->mMongoComponent = nullptr;
    }

    bool MongoDB::Awake()
    {
        this->mApp->AddComponent<MongoDBComponent>();
		return true;
    }

	bool MongoDB::OnInit()
    {
        BIND_COMMON_RPC_METHOD(MongoDB::Query);
        BIND_COMMON_RPC_METHOD(MongoDB::Insert);
        BIND_COMMON_RPC_METHOD(MongoDB::Delete);
        BIND_COMMON_RPC_METHOD(MongoDB::Update);
        BIND_COMMON_RPC_METHOD(MongoDB::SetIndex);
        BIND_COMMON_RPC_METHOD(MongoDB::RunCommand);
        this->mMongoComponent = this->GetComponent<MongoDBComponent>();
        return true;
    }

	bool MongoDB::OnStart()
	{
		for(int index = 0; index < MongoConfig::Inst()->MaxCount; index++)
		{
			this->mMongoComponent->MakeMongoClient();
		}
		return this->mMongoComponent->Ping(0);
	}

    void MongoDB::OnClose()
    {
        this->WaitAllMessageComplete(); //等待所有任务完成
        this->mMongoComponent->CloseClients();
    }

    int MongoDB::RunCommand(const db::mongo::command::request &request, db::mongo::command::response &response)
    {
        std::shared_ptr<CommandRequest> mongoRequest
                = std::make_shared<CommandRequest>();
        if (!mongoRequest->document.FromByJson(request.json()))
        {
            return XCode::CallArgsError;
        }
        TcpMongoClient * mongoClient = this->mMongoComponent->GetClient();
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

    int MongoDB::Insert(const db::mongo::insert &request)
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
		TcpMongoClient * mongoClient = this->mMongoComponent->GetClient(request.flag());
		std::shared_ptr<CommandResponse> response = this->mMongoComponent->Run(mongoClient, mongoRequest);
		if (response == nullptr || response->GetDocumentSize() <= 0)
		{
#ifdef __DEBUG__
			LOG_ERROR("insert [" << tab << "] failure json =" << request.json());
#endif
			return XCode::Failure;
		}
        int res = 0;
        Bson::Reader::Document & result = response->Get();
        if(result.Get("n", res) && res > 0)
        {
            return XCode::Successful;
        }
        return XCode::Failure;
	}

    int MongoDB::Delete(const db::mongo::remove &request)
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

        Bson::Writer::Document delDocument;
        delDocument.Add("q", document);
        delDocument.Add("limit", request.limit());

        Bson::Writer::Array documentArray(delDocument);

        mongoRequest->document.Add("delete", tab);
        mongoRequest->document.Add("deletes", documentArray);
        TcpMongoClient * mongoClient = this->mMongoComponent->GetClient(request.flag());
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
    int MongoDB::Update(const db::mongo::update &request)
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
        TcpMongoClient * mongoClient = this->mMongoComponent->GetClient(request.flag());
        std::shared_ptr<CommandResponse> response = this->mMongoComponent->Run(mongoClient, mongoRequest);
        if (response == nullptr || response->GetDocumentSize() == 0)
        {
            return XCode::Failure;
        }
        int count = 0;
        Bson::Reader::Document &result = response->Get();
        return (result.Get("n", count) && count > 0) ? XCode::Successful : XCode::Failure;
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
        return this->mMongoComponent->SetIndex(tab, name) ? XCode::Successful : XCode::Failure;
    }

    int MongoDB::Query(const db::mongo::query::request &request, db::mongo::query::response &response)
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
        TcpMongoClient * mongoClient = this->mMongoComponent->GetClient();
        std::shared_ptr<CommandResponse> queryResponse = this->mMongoComponent->Run(mongoClient, mongoRequest);

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

}