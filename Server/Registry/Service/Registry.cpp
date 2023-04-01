//
// Created by zmhy0073 on 2022/10/25.
//

#include"Registry.h"
#include"Entity/App/App.h"
#include"Common/Service/Node.h"
#include"Message/db.pb.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Rpc/Component/InnerNetComponent.h"
#include"Rpc/Component/NodeMgrComponent.h"

#include"Proto/Component/ProtoComponent.h"

#ifdef __ENABLE_MYSQL__
#include"Mysql/Client/MysqlMessage.h"
#include"Mysql/Component/MysqlDBComponent.h"
#else
#include"Util/String/StringHelper.h"
#include"Sqlite/Component/SqliteComponent.h"
#endif
namespace Sentry
{
	Registry::Registry()
	{
		this->mIndex = 0;
		this->mNodeComponent = nullptr;
		this->mInnerComponent = nullptr;
#ifdef __ENABLE_MYSQL__
		this->mMysqlComponent = nullptr;
#else
		this->mDatabaseIndex = 0;
		this->mSqliteComponent = nullptr;
#endif

	}

	bool Registry::Awake()
	{
#ifndef __ENABLE_MYSQL__
		this->mApp->AddComponent<SqliteComponent>();
#else
		this->mApp->AddComponent<MysqlDBComponent>();
#endif
		return true;
	}

	bool Registry::OnInit()
	{
		BIND_COMMON_RPC_METHOD(Registry::Ping);
		BIND_COMMON_RPC_METHOD(Registry::Query);
		BIND_ADDRESS_RPC_METHOD(Registry::Register);
		BIND_ADDRESS_RPC_METHOD(Registry::UnRegister);
		this->mNodeComponent = this->GetComponent<NodeMgrComponent>();
		this->mInnerComponent = this->GetComponent<InnerNetComponent>();
		return true;
	}

    bool Registry::OnStart()
	{
		ProtoComponent* component = this->GetComponent<ProtoComponent>();
		LOG_CHECK_RET_FALSE(component->Import("mysql/server.proto"));
		std::shared_ptr<Message> message = component->New("server.registry");
		if(message == nullptr)
		{
			LOG_ERROR("create protobuf type [server.registry] error");
			return false;
		}
#ifdef __ENABLE_MYSQL__
		this->mTable = message->GetTypeName();
		this->mMysqlComponent = this->GetComponent<MysqlDBComponent>();
		LOG_CHECK_RET_FALSE(this->mMysqlComponent != nullptr);
        this->mIndex = this->mMysqlComponent->MakeMysqlClient();
		std::vector<std::string> keys{ "rpc_address" };
		std::shared_ptr<Mysql::CreateTabCommand> command =
                std::make_shared<Mysql::CreateTabCommand>(this->mTable,message, keys);
        return this->mMysqlComponent->Run(this->mIndex , command)->IsOk();
#else
		std::vector<std::string> ret;
		std::string name = message->GetTypeName();
		Helper::Str::Split(name, ".", ret);

		this->mTable = std::move(ret[1]);
		const std::string key("rpc_address");
		this->mSqliteComponent = this->GetComponent<SqliteComponent>();
		this->mDatabaseIndex = this->mSqliteComponent->Open(ret[0]);
		return this->mSqliteComponent->MakeTable(this->mDatabaseIndex, key, *message);
#endif
	}

	int Registry::Query(const com::type::string& request, s2s::server::list& response)
	{
		std::stringstream sqlStream;
		const std::string& name = request.str();
		sqlStream << "select * from " << this->mTable;
		if (!name.empty())
		{
			sqlStream << " where server_name='" << name << "'";
		}
		sqlStream << ";";
		const std::string sql = sqlStream.str();
#ifdef __ENABLE_MYSQL__
		std::shared_ptr<Mysql::QueryCommand> command
			= std::make_shared<Mysql::QueryCommand>(sql);

		std::shared_ptr<Mysql::Response> response1 =
			this->mMysqlComponent->Run(this->mIndex, command);
		const std::vector<std::string> & result = response1->Array();
#else
		std::vector<std::string> result;
		this->mSqliteComponent->Query(this->mDatabaseIndex, sql.c_str(), result);
#endif
		for (size_t index = 0; index < result.size(); index++)
		{
			Json::Reader document;
			CONSOLE_LOG_INFO(result[index]);
			const std::string& json = result.at(index);
			if (!document.ParseJson(result.at(index)))
			{
				return XCode::ParseJsonFailure;
			}
			long long time = 0;
			long long now = Helper::Time::NowSecTime();
			document.GetMember("last_ping_time", time);
			if (now - time <= 15)
			{
				s2s::server::info* message = response.add_list();
				document.GetMember("server_name", *message->mutable_name());
				document.GetMember("rpc_address", *message->mutable_rpc());
				document.GetMember("http_address", *message->mutable_http());
			}
		}
		return XCode::Successful;
	}

	int Registry::Register(const std::string& address, const s2s::server::info& request)
	{
		const std::string& rpc = request.rpc();
		const std::string& http = request.http();
		const std::string& server = request.name();
		LOG_ERROR_RETURN_CODE(!rpc.empty(), XCode::CallArgsError);
		LOG_ERROR_RETURN_CODE(!server.empty(), XCode::CallArgsError);
        std::stringstream st;
        st << "replace into " << this->mTable << " (server_name,rpc_address,";
        st << "http_address,last_ping_time,last_time_str) values('" << server <<"','" << rpc;
        st << "','" << http << "'," << Helper::Time::NowSecTime() <<",'" << Helper::Time::GetDateString() << "');";
		const std::string sql = st.str();
#ifdef __ENABLE_MYSQL__
		if (!this->mMysqlComponent->Execute(this->mIndex, std::make_shared<Mysql::SqlCommand>(sql)))
		{
			return XCode::SaveToMysqlFailure;
		}
#else
		if (!this->mSqliteComponent->Exec(this->mDatabaseIndex, sql.c_str()))
		{
			return XCode::SaveToMysqlFailure;
		}
#endif
		const std::string func("Join");
		this->mRegistryServers.insert(address);
		this->mNodeComponent->AddRpcServer(server, rpc);
		this->mNodeComponent->AddHttpServer(server, http);

		RpcService* rpcService = this->mApp->GetService<Node>();
		for (const std::string& server : this->mRegistryServers)
		{
			rpcService->Send(server, func, request);
			LOG_INFO("send server join message to " << server);
		}
		return XCode::Successful;
	}

	int Registry::Ping(const Rpc::Packet& packet)
	{
		const std::string & address = packet.From();
		const ServiceNodeInfo * nodeInfo = this->mInnerComponent->GetSeverInfo(address);
		if(nodeInfo != nullptr)
		{
            std::stringstream st;
            long long time = Helper::Time::NowSecTime();
			const std::string table("server.registry");
			const std::string & rpc = nodeInfo->RpcAddress;
            st << "update " << this->mTable << "SET last_ping_time=" << time;
            st << ",last_time_str='" << Helper::Time::GetDateString() << "' where rpc_address='" << rpc << "'";
            const std::string sql = st.str();
#ifdef __ENABLE_MYSQL__
			if(!this->mMysqlComponent->Execute(this->mIndex, std::make_shared<Mysql::SqlCommand>(sql)))
            {
                return XCode::SaveToMysqlFailure;
            }
#else
			if(!this->mSqliteComponent->Exec(this->mDatabaseIndex, sql.c_str()))
			{
				return XCode::SaveToMysqlFailure;
			}
#endif
		}
		return XCode::Successful;
	}

	int Registry::UnRegister(const std::string& address, const com::type::string& request)
	{
		const std::string& rpc = request.str();
		LOG_ERROR_RETURN_CODE(!rpc.empty(), XCode::CallArgsError);
		if (!this->mNodeComponent->DelServer(rpc))
		{
			return XCode::Failure;
		}
		RpcService* rpcService = this->mApp->GetService<Node>();
		const std::string sql = fmt::format("delete from {0} where rpc_address='{1}'", this->mTable, rpc);
#ifdef __ENABLE_MYSQL__
		if (!this->mMysqlComponent->Run(this->mIndex, std::make_shared<Mysql::SqlCommand>(sql))->IsOk())
		{
			return XCode::SaveToMysqlFailure;
		}
#else
		if(!this->mSqliteComponent->Exec(this->mDatabaseIndex, sql.c_str()))
		{
			return XCode::SaveToMysqlFailure;
		}
#endif
		const std::string func("Exit");
		auto iter = this->mRegistryServers.find(address);
		if (iter != this->mRegistryServers.end())
		{
			this->mRegistryServers.erase(iter);
		}
		for (const std::string& target : this->mRegistryServers)
		{
			rpcService->Send(target, func, request);
		}
		return XCode::Successful;
	}

	void Registry::OnSecondUpdate(int tick)
	{
		if (tick % 10 != 0) return;
		long long now = Helper::Time::NowSecTime();
	}
}