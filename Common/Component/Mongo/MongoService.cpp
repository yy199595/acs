//
// Created by mac on 2022/5/19.
//

#include"MongoService.h"
#include"DB/Mongo/MongoClient.h"
#include"Component/Scene/NetThreadComponent.h"
namespace Sentry
{
	bool MongoService::OnStartService(ServiceMethodRegister& methodRegister)
	{
		return true;
	}

	bool MongoService::LateAwake()
	{
		return true;
	}

	bool MongoService::OnStart()
	{
		NetThreadComponent* threadComponent = this->GetComponent<NetThreadComponent>();
		IAsioThread& asioThread = threadComponent->AllocateNetThread();
		std::shared_ptr<SocketProxy> socketProxy(new SocketProxy(asioThread, "127.0.0.1", 27017));
		std::shared_ptr<Mongo::MongoClientContext> mongoClientContext(new Mongo::MongoClientContext(socketProxy));
		if (mongoClientContext->StartConnect())
		{
			LOG_DEBUG("连接mongo成功");
		}

		std::shared_ptr<Mongo::MongoInsertRequest> insertRequest
				= std::make_shared<Mongo::MongoInsertRequest>();
		insertRequest->header.responseTo = 1;
		insertRequest->collectionName = "db.test";
		insertRequest->document.set("_id", 100);
		insertRequest->document.set("name", "xiaoming");
		insertRequest->document.set("age", 20);
		mongoClientContext->SendMongoCommand(insertRequest);

		return true;
	}
}