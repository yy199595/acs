//
// Created by zmhy0073 on 2022/7/16.
//
#ifdef __ENABLE_MYSQL__
#ifndef SERVER_MYSQLRPCCOMPONENT_H
#define SERVER_MYSQLRPCCOMPONENT_H

#include"Core/Queue/Queue.h"
#include"Mysql/Client/MysqlDefine.h"
#include"Mysql/Client/MysqlMessage.h"
#include"Util/Guid/NumberBuilder.h"
#include "Mysql/Config/MysqlConfig.h"
#include"Rpc/Component/RpcTaskComponent.h"

struct lua_State;
namespace acs
{
	class MysqlTask : public IRpcTask<Mysql::Response>, protected WaitTaskSourceBase
    {
    public:
        explicit MysqlTask(int taskId);
    public:
        inline Mysql::Response * Await();
		inline void OnResponse(Mysql::Response * response) final;
	private:
        Mysql::Response * mMessage;
    };

	inline Mysql::Response* MysqlTask::Await()
	{
		this->YieldTask();
		return this->mMessage;
	}

	inline void MysqlTask::OnResponse(Mysql::Response* response)
	{
		this->mMessage = response;
		this->ResumeTask();
	}
}

namespace acs
{
    class MysqlClient;
	class MysqlDBComponent final : public RpcTaskComponent<int, Mysql::Response>,
		public IRpc<Mysql::IRequest, Mysql::Response>, public ILuaRegister, public IDestroy
    {
	public:
		bool Execute(std::unique_ptr<Mysql::IRequest> command);
		bool SyncExecute(std::unique_ptr<Mysql::IRequest> command);
		Mysql::Response * Run(std::unique_ptr<Mysql::IRequest> command);
		bool Send(std::unique_ptr<Mysql::IRequest> command, int & rpcId);
		const mysql::MysqlConfig & Config() const { return this->mConfig; }
		std::unique_ptr<Mysql::Response> SyncRun(std::unique_ptr<Mysql::IRequest> command);
	 private:
		bool Awake() final;
		bool LateAwake() final;
		void OnDestroy() final;
		void OnLuaRegister(Lua::ModuleClass &luaRegister) final;
		void OnMessage(Mysql::IRequest *request, Mysql::Response * message) final;
	private:
		mysql::MysqlConfig mConfig;
		math::NumberPool<int> mNumPool;
		custom::Queue<MysqlClient *> mMysqlClients;
    };
}


#endif //SERVER_MYSQLRPCCOMPONENT_H
#endif