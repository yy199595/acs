//
// Created by zmhy0073 on 2022/10/25.
//

#include"Registry.h"

#include"Entity/Unit/App.h"
#include"Common/Service/Node.h"
#include"Util/String/StringHelper.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Rpc/Component/LocationComponent.h"
#include"Proto/Component/ProtoComponent.h"
#include"Redis/Component/RedisComponent.h"
#include"Redis/Component/RedisLuaComponent.h"
#include"google/protobuf/util/json_util.h"

#include"Message/mysql/server.pb.h"
#include"Mysql/Client/MysqlMessage.h"
#include"Mysql/Component/MysqlDBComponent.h"
#include "Util/Sql/SqlHelper.h"

namespace Tendo
{
	Registry::Registry()
	{
		this->mMysqlComponent = nullptr;
	}

	bool Registry::Awake()
	{
		this->mApp->AddComponent<RedisComponent>();
		this->mApp->AddComponent<MysqlDBComponent>();
		return true;
	}

	bool Registry::OnInit()
	{
		BIND_COMMON_RPC_METHOD(Registry::Ping);
		BIND_COMMON_RPC_METHOD(Registry::Query);
		BIND_ADDRESS_RPC_METHOD(Registry::Register);
		BIND_COMMON_RPC_METHOD(Registry::UnRegister);
		this->mMysqlComponent = this->GetComponent<MysqlDBComponent>();
		return true;
	}

	bool Registry::OnStart()
	{
		std::shared_ptr<server::registry> message = std::make_shared<server::registry>();
		this->mTable = message->GetTypeName();
		std::vector<std::string> keys {"rpc_address", "server_name" };
		std::shared_ptr<Mysql::ICommand> command =
				std::make_shared<Mysql::CreateTabCommand>(this->mTable, message, keys);
		return this->mMysqlComponent->Execute(command);
	}

	int Registry::Query(const com::type::string& request, s2s::server::list& response)
	{
		const std::string & server = request.str();
		LOG_ERROR_RETURN_CODE(!server.empty(), XCode::CallArgsError);
		std::string sql = fmt::format("SELECT * from {0} where server_name='{1}'", this->mTable, server);
		std::shared_ptr<Mysql::ICommand> queryCommand = std::make_shared<Mysql::QueryCommand>(sql);
		std::shared_ptr<Mysql::Response> mysqlResponse = this->mMysqlComponent->Run(queryCommand);
		if(!mysqlResponse->IsOk())
		{
			return XCode::MysqlInvokeFailure;
		}
		for(size_t index = 0; index < mysqlResponse->ArraySize(); index++)
		{
			server::registry message;
			const std::string & json = mysqlResponse->Get(index);
			if(!util::JsonStringToMessage(json, &message).ok())
			{
				return XCode::JsonCastProtoFailure;
			}
			s2s::server::info * info = response.add_list();
			{
				info->set_server_id(message.server_id());
				info->set_server_name(message.server_name());
				info->set_group_id(message.server_group_id());
				info->mutable_listens()->insert({"rpc", message.rpc_address()});
				info->mutable_listens()->insert({"http", message.http_address()});
				info->mutable_listens()->insert({"gate", message.gate_address()});
			}
		}
		return XCode::Successful;
	}

	int Registry::Register(const std::string & address, const s2s::server::info& request)
	{
		if(address.find("tcp") != 0)
		{
			return XCode::OnlyUseTcpProtocol;
		}
		server::registry message;
		auto iter1 = request.listens().find("rpc");
		auto iter2 = request.listens().find("http");
		auto iter3 = request.listens().find("gate");
		if(iter1 != request.listens().end())
		{
			message.set_rpc_address(iter1->second);
		}
		if(iter2 != request.listens().end())
		{
			message.set_http_address(iter2->second);
		}
		if(iter3 != request.listens().end())
		{
			message.set_gate_address(iter3->second);
		}
		message.set_server_id(request.server_id());
		message.set_server_name(request.server_name());
		message.set_server_group_id(request.group_id());
		message.set_last_ping_time(Helper::Time::NowSecTime());
		message.set_last_time_str(Helper::Time::GetDateString());

		std::string sql;
		SqlHelper sqlHelper;
		if(!sqlHelper.Replace(message,sql))
		{
			return XCode::CallArgsError;
		}
		std::shared_ptr<Mysql::SqlCommand> command
			= std::make_shared<Mysql::SqlCommand>(sql);
		if(!this->mMysqlComponent->Execute(command))
		{
			return XCode::SaveToMysqlFailure;
		}
		this->mServers.emplace(address, request.New());
		RpcService * rpcService = this->mApp->GetService<Node>();
		for(auto iter = this->mServers.begin(); iter != this->mServers.end(); iter++)
		{
			rpcService->Send(iter->first, "Join", request);
		}
		return XCode::Successful;
	}

	int Registry::Ping(const Msg::Packet& packet)
	{
		
		return XCode::Successful;
	}

	int Registry::UnRegister(const com::type::int32& request)
	{

		return XCode::Successful;
	}

	void Registry::OnSecondUpdate(int tick)
	{

	}

	void Registry::Invoke(const DisConnectEvent* message)
	{
		const std::string & address = message->Addr;
		auto iter = this->mServers.find(address);
		if(iter == this->mServers.end())
		{
			return;
		}
		const s2s::server::info * info = iter->second;
		this->mServers.erase(iter);
		LOG_WARN(info->server_name() << " [" << address << "] disconnect ");
		{
			s2s::server::info request;
			request.set_server_id(info->server_id());
			request.set_server_name(info->server_name());
			RpcService * rpcService = this->mApp->GetService<Node>();
			for(auto iter = this->mServers.begin(); iter != this->mServers.end(); iter++)
			{
				rpcService->Send(iter->first, "Exit", request);
			}
		}
		delete info;
	}
}