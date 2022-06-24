//
// Created by mac on 2022/5/19.
//

#include"MongoService.h"
#include"DB/Mongo/MongoClient.h"
#include"Script/Extension/Mongo/LuaMongo.h"
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
		luaRegister.BeginRegister<MongoService>();
		luaRegister.PushExtensionFunction("Run", Lua::MongoEx::Run);
	}

	bool MongoService::OnStart()
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

			LuaScriptComponent * luaScriptComponent = this->GetComponent<LuaScriptComponent>();

			lua_State * lua = luaScriptComponent->GetLuaEnv();


			std::shared_ptr<Mongo::MongoQueryRequest> queryRequest
				= std::make_shared<Mongo::MongoQueryRequest>();
            queryRequest->collectionName = "admin.$cmd";

			queryRequest->header.requestID = 1;
			Lua::lua_getfunction(lua, "Main", "Bson");
			if(lua_pcall(lua, 0, 1, 0) != LUA_OK)
			{
				LOG_ERROR(lua_tostring(lua, -1));
			}
			size_t size = 0;
			queryRequest->command.append(luaL_tolstring(lua, -1, &size), size);

			//queryRequest->document.set("hello", 1);

			mongoClientContext->SendMongoCommand(queryRequest);
		}
		return true;
	}
}