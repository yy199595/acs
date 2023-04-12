//
// Created by zmhy0073 on 2022/7/16.
//
#ifdef __ENABLE_MYSQL__
#ifndef SERVER_MYSQLRPCCOMPONENT_H
#define SERVER_MYSQLRPCCOMPONENT_H

#include"Mysql/Client/MysqlDefine.h"
#include"Mysql/Client/MysqlMessage.h"
#include"Util/Guid/NumberBuilder.h"
#include"Rpc/Component/RpcTaskComponent.h"
#include "Mysql/Config/MysqlConfig.h"

struct lua_State;
namespace Tendo
{
    class MysqlTask : public IRpcTask<Mysql::Response>
    {
    public:
        explicit MysqlTask(int taskId);
    public:
        void OnResponse(std::shared_ptr<Mysql::Response> response) final;
        std::shared_ptr<Mysql::Response> Await() { return mTask.Await(); }
    private:
        TaskSource<std::shared_ptr<Mysql::Response>> mTask;
    };
}

namespace Tendo
{
    class MysqlClient;
	class MysqlDBComponent : public RpcTaskComponent<int, Mysql::Response>,
							 public IRpc<Mysql::Response>, public ILuaRegister, public IDestroy, public ISecondUpdate
    {
	public:
		bool Ping(int index = 0);
		bool GetClientHandle(int& id);
		const MysqlConfig & Config() const { return this->mConfig; }
		bool Execute(const std::shared_ptr<Mysql::ICommand>& command);
		bool Execute(int index, const std::shared_ptr<Mysql::ICommand>& command);
		bool Send(int index, const std::shared_ptr<Mysql::ICommand> & command, int & rpcId);
		std::shared_ptr<Mysql::Response> Run(const std::shared_ptr<Mysql::ICommand>& command);
		std::shared_ptr<Mysql::Response> Run(int index, const std::shared_ptr<Mysql::ICommand>& command);
	 private:
		bool Awake() final;
		bool LateAwake() final;
		void OnDestroy() final;
		void OnSecondUpdate(int tick) final;
		void OnLuaRegister(Lua::ClassProxyHelper &luaRegister) final;
		void OnConnectSuccessful(const std::string &address) final;
		void OnMessage(std::shared_ptr<Mysql::Response> message) final;
	private:
		MysqlConfig mConfig;
		std::queue<int> mAllotQueue;
		std::unique_ptr<SqlHelper> mSqlHelper;
		std::shared_ptr<MysqlClient> mMainClient;
		std::unordered_map<int, std::shared_ptr<MysqlClient>> mMysqlClients;
    };
}


#endif //SERVER_MYSQLRPCCOMPONENT_H
#endif