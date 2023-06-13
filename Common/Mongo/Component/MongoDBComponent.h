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
    	int MakeMongoClient(const Mongo::MongoConfig & config);
        bool SetIndex(int id, const std::string & tab, const std::string & name);
    public:
		bool GetClientHandler(int & id);
        void OnClientError(int index, int code);
		unsigned int GetWaitCount() const { return this->mWaitCount; }
	public:
        bool Send(int id, const std::shared_ptr<Mongo::CommandRequest>& request, int & taskId);
		std::shared_ptr<Mongo::CommandResponse> Run(int id, const std::shared_ptr<Mongo::CommandRequest>& request);
	 private:
		void OnDestroy() final;
    	Mongo::TcpMongoClient* GetClient(int index = -1);
		void OnLuaRegister(Lua::ClassProxyHelper& luaRegister) final;
		void OnConnectSuccessful(const std::string &address) final;
		void OnMessage(std::shared_ptr<Mongo::CommandResponse> message) final;
	private:
    	int mIndex;
		unsigned int mWaitCount;
        std::unordered_map<int, std::shared_ptr<Mongo::TcpMongoClient>> mMongoClients;
    };
}


#endif //SERVER_MONGORPCCOMPONENT_H
