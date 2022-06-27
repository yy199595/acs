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

	void MongoService::OnLuaRegister(Lua::ClassProxyHelper& luaRegister)
	{

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

			LuaScriptComponent * luaScriptComponent = this->GetComponent<LuaScriptComponent>();

			lua_State * lua = luaScriptComponent->GetLuaEnv();


			std::shared_ptr<Mongo::MongoQueryRequest> queryRequest
				= std::make_shared<Mongo::MongoQueryRequest>();
            queryRequest->collectionName = "admin.$cmd";

			queryRequest->header.requestID = 1;
			queryRequest->flag = 0;
			queryRequest->numberToSkip = 0;
			queryRequest->numberToReturn = 1;
			queryRequest->document.Add("ismaster", 1);

			mongoClientContext->SendMongoCommand(queryRequest);
		}
		return true;
	}
}