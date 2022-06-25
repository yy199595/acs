//
// Created by mac on 2022/5/19.
//

#include"MongoService.h"
#include"DB/Mongo/MongoProto.h"
#include"DB/Mongo/Bson/MongoBson.h"
#include"DB/Mongo/MongoClient.h"
#include"Script/Extension/Mongo/LuaMongo.h"
#include"Component/Scene/NetThreadComponent.h"
namespace Sentry
{
	bool MongoService::OnStartService(ServiceMethodRegister& methodRegister)
	{
		std::shared_ptr<Mongo::MongoQueryRequest> queryRequest
			= std::make_shared<Mongo::MongoQueryRequest>();
		queryRequest->collectionName = "admin.$cmd";

		Bson::BsonDocumentNode * bsonDocumentNode = new Bson::BsonDocumentNode();
		bsonDocumentNode->Add("_id", 444);
		bsonDocumentNode->Add("name", "yjz");
		bsonDocumentNode->Add("age", 10);
		queryRequest->document.Add("insert", "UserLevelData");
		queryRequest->document.Add("documents", bsonDocumentNode);

		asio::streambuf streambuf1;
		std::ostream oss(&streambuf1);
		std::iostream iss(&streambuf1);
		size_t size = queryRequest->document.GetSize();
		oss.write((char *)&size, sizeof(int));
		queryRequest->document.Serialize(oss);
		lua_State * lua = this->GetComponent<LuaScriptComponent>()->GetLuaEnv();

		int num = streambuf1.size();
		iss.read((char *)&num, sizeof(int));

		Bson::BsonDocumentNode * bsonDocumentNode2 = new Bson::BsonDocumentNode(iss, num);



		size_t ss = queryRequest->document.GetSize();
		if(Lua::lua_getfunction(lua, "Main", "Bson"))
		{
			std::string bson;
			char buffer[128] = { 0};
			size_t size = iss.readsome(buffer, 128);
			while(size > 0)
			{
				bson.append(buffer, size);
				size = iss.readsome(buffer, 128);
			}
			size_t s = bson.size();
			lua_pushlstring(lua, bson.c_str(), bson.size());
			if(lua_pcall(lua, 1, 0, 0) != LUA_OK)
			{
				LOG_ERROR(lua_tostring(lua, -1));
			}
		}

		return true;
	}

	bool MongoService::LateAwake()
	{
		return true;
	}

	void MongoService::OnLuaRegister(Lua::ClassProxyHelper& luaRegister)
	{
		luaRegister.BeginRegister<MongoService>();
		luaRegister.PushExtensionFunction("Run", Lua::MongoEx::Run);
	}

	void MongoService::OnAllServiceStart()
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

			std::shared_ptr<Mongo::MongoQueryRequest> queryRequest
				= std::make_shared<Mongo::MongoQueryRequest>();
            queryRequest->collectionName = "admin.$cmd";

			queryRequest->header.requestID = 1;
			queryRequest->flag = 0;
			queryRequest->numberToSkip = 0;
			queryRequest->numberToReturn = 1;
			queryRequest->document.Add("ismaster",1);

			mongoClientContext->SendMongoCommand(queryRequest);
		}
	}
}