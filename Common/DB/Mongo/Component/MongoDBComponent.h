//
// Created by mac on 2022/6/28.
//

#ifndef APP_MONGODBCOMPONENT_H
#define APP_MONGODBCOMPONENT_H
#include"Util/Tools/NumberBuilder.h"
#include"Mongo/Client/MongoProto.h"
#include"Mongo/Client/MongoClient.h"
#include"Proto/Document/BsonDocument.h"
#include"Util/Tools/NumberBuilder.h"
#include"Mongo/Config/MongoConfig.h"
#include"Core/Queue/Queue.h"
#include"Rpc/Component/RpcTaskComponent.h"

namespace acs
{
	class MongoTask : public IRpcTask<mongo::Response>, protected WaitTaskSourceBase
	{
	public:
		explicit MongoTask(int taskId);
    public:
		inline mongo::Response * Await();
		inline void OnResponse(mongo::Response * response) final;
	private:
		mongo::Response * mMessage;
	};

	inline mongo::Response* MongoTask::Await()
	{
		this->YieldTask();
		return this->mMessage;
	}

	inline void MongoTask::OnResponse(mongo::Response* response)
	{
		this->mMessage = response;
		this->ResumeTask();
	}
}

namespace acs
{
	class LuaMongoTask : public IRpcTask<mongo::Response>
	{
	public:
		LuaMongoTask(lua_State * lua, int id);
		~LuaMongoTask() final;
	public:
		int Await();
		void OnResponse(mongo::Response * response) final;
	private:
		int mRef;
		lua_State * mLua;
	};
}

namespace acs
{
    class MongoDBComponent : public RpcTaskComponent<int,mongo::Response>,
			public IRpc<mongo::Request, mongo::Response>,
			public IDestroy, public IServerRecord, public ILuaRegister
	{
	public:
		MongoDBComponent() = default;
		~MongoDBComponent() final = default;
	public:
		mongo::Response * Run(std::unique_ptr<mongo::Request> request);
		void LuaSend(std::unique_ptr<mongo::Request> request, int & taskId);
		mongo::Response * Run(const std::string & db, std::unique_ptr<mongo::Request> request);
	private:
		bool Awake() final;
		bool LateAwake() final;
		void OnDestroy() final;
		void OnRecord(json::w::Document &document) final;
		void OnLuaRegister(Lua::ModuleClass &luaRegister) final;
		void OnMessage(int id, mongo::Request * request, mongo::Response * message) final;
	private:
		void Send(std::unique_ptr<mongo::Request> request);
		void Send(int id, std::unique_ptr<mongo::Request> request);
	private:
		std::string mCommand;
		mongo::MongoConfig mConfig;
		custom::Queue<int> mFreeClients;
		math::NumberPool<int> mNumberPool;
		std::queue<std::unique_ptr<mongo::Request>> mRequests;
		std::unordered_map<int, mongo::Client *> mMongoClients;
	};
}


#endif //APP_MONGODBCOMPONENT_H
