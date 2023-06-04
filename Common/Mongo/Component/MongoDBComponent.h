//
// Created by mac on 2022/6/28.
//

#ifndef SERVER_MONGORPCCOMPONENT_H
#define SERVER_MONGORPCCOMPONENT_H
#include"Util/Guid/NumberBuilder.h"
#include"Mongo/Client/MongoProto.h"
#include"Mongo/Client/MongoClient.h"
#include"Mongo/Client/BsonDocument.h"
#include"Util/Guid/NumberBuilder.h"
#include"Mongo/Config/MongoConfig.h"
#include"Rpc/Component/RpcTaskComponent.h"

using namespace Mongo;

namespace Tendo
{
	class MongoTask : public IRpcTask<Mongo::CommandResponse>
	{
	public:
		explicit MongoTask(int taskId);
    public:
		void OnResponse(std::shared_ptr<Mongo::CommandResponse> response) final;
		std::shared_ptr<Mongo::CommandResponse> Await() { return mTask.Await(); }
	private:
		TaskSource<std::shared_ptr<Mongo::CommandResponse>> mTask;
	};
}


namespace Tendo
{
    class MongoDBComponent : public RpcTaskComponent<int,Mongo::CommandResponse>, 
							 public IRpc<Mongo::CommandResponse>, public ILuaRegister, public IDestroy
	{
	public:
		MongoDBComponent();
		~MongoDBComponent() final = default;
	public:
        bool Ping(int id);
        bool SetIndex(const std::string & tab, const std::string & name);
		inline const MongoConfig & Config() const { return this->mConfig; }
    public:
		bool GetClientHandler(int & id);
        void OnClientError(int index, int code);
		unsigned int GetWaitCount() const { return this->mWaitCount; }
	public:
        bool Send(int id, const std::shared_ptr<CommandRequest>& request, int & taskId);
		std::shared_ptr<Mongo::CommandResponse> Run(int id, const std::shared_ptr<CommandRequest>& request);
	 private:
		bool Awake() final;
		void OnDestroy() final;
		bool LateAwake() final;
		TcpMongoClient* GetClient(int index = -1);
		void OnLuaRegister(Lua::ClassProxyHelper& luaRegister) final;
		void OnConnectSuccessful(const std::string &address) final;
		void OnMessage(std::shared_ptr<CommandResponse> message) final;
	private:
		MongoConfig mConfig;
		unsigned int mWaitCount;
		std::queue<int> mAllotQueue;
        std::unordered_map<int, std::shared_ptr<TcpMongoClient>> mMongoClients;
    };
}


#endif //SERVER_MONGORPCCOMPONENT_H
