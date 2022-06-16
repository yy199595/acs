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
#ifdef ONLY_MAIN_THREAD
		IAsioThread& asioThread = this->GetApp()->GetTaskScheduler();
#else
		NetThreadComponent* threadComponent = this->GetComponent<NetThreadComponent>();
		IAsioThread& asioThread = threadComponent->AllocateNetThread();
#endif
		std::shared_ptr<SocketProxy> socketProxy(new SocketProxy(asioThread, "114.115.167.51", 27017));
		std::shared_ptr<Mongo::MongoClientContext> mongoClientContext(new Mongo::MongoClientContext(socketProxy));
		if (mongoClientContext->StartConnect())
		{
			LOG_DEBUG("连接mongo成功");

			std::shared_ptr<Mongo::MongoInsertRequest> insertRequest
				= std::make_shared<Mongo::MongoInsertRequest>();
			insertRequest->collectionName = "test.user";
			insertRequest->document.set("age", 20);
			insertRequest->document.set("_id", 100);
			insertRequest->document.set("name", "xiaoming");
			mongoClientContext->SendMongoCommand(insertRequest);

		}
		return true;
	}
}