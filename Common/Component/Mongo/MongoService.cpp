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

	void MongoService::OnAllServiceStart()
	{
#ifdef ONLY_MAIN_THREAD
		IAsioThread& asioThread = this->GetApp()->GetTaskScheduler();
#else
		NetThreadComponent* threadComponent = this->GetComponent<NetThreadComponent>();
		IAsioThread& asioThread = threadComponent->AllocateNetThread();
#endif
		std::shared_ptr<SocketProxy> socketProxy(new SocketProxy(asioThread, "127.0.0.1", 27017));
		std::shared_ptr<Mongo::MongoClientContext> mongoClientContext(new Mongo::MongoClientContext(socketProxy));
		if (mongoClientContext->StartConnect())
		{
			LOG_DEBUG("连接mongo成功");

//            std::shared_ptr<Mongo::MongoQueryRequest> queryRequest
//                    = std::make_shared<Mongo::MongoQueryRequest>();
//            queryRequest->collectionName = "ET.UserLevelData.$cmd";
//            queryRequest->document.set("_id", 444);

			std::shared_ptr<Mongo::MongoQueryRequest> queryRequest
				= std::make_shared<Mongo::MongoQueryRequest>();
            queryRequest->collectionName = "admin.$cmd";

//            minibson::document document1;
//            minibson::document document11;
//            document11.set("name", "robo3t-1.4.4");
//            document1.set("application", document11);
//
//            minibson::document document2;
//            minibson::document document22;
//            document22.set("name", "MongoDB Internal Client");
//            document22.set("version", "4.2.6-18-g6cdb6ab");
//            document2.set("driver", document22);
//
//            minibson::document document3;
//            minibson::document document33;
//            document33.set("architecture", "x86_64");
//            document33.set("name", "Mac OS X");
//            document33.set("type", "Darwin");
//            document33.set("version", "21.4.0");
//
//            document3.set("os", document33);
//
//            minibson::document document4;
//
//
//            queryRequest->document.set("client", document1);
            queryRequest->document.set("hello", 1);


			mongoClientContext->SendMongoCommand(queryRequest);

		}
	}
}