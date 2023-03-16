//
// Created by zmhy0073 on 2022/10/25.
//

#include"Registry.h"
#include"Service/Node.h"
#include"Message/db.pb.h"
#include"Component/InnerNetComponent.h"
#include"Component/NodeMgrComponent.h"
#include"Component/MysqlDBComponent.h"
#include"Component/ProtoComponent.h"
#include"Client/MysqlMessage.h"
namespace Sentry
{
    bool Registry::OnStart()
	{
		BIND_COMMON_RPC_METHOD(Registry::Ping);
		BIND_COMMON_RPC_METHOD(Registry::Query);
		BIND_COMMON_RPC_METHOD(Registry::Register);
		BIND_COMMON_RPC_METHOD(Registry::UnRegister);
		this->mNodeComponent = this->GetComponent<NodeMgrComponent>();
		this->mInnerComponent = this->GetComponent<InnerNetComponent>();
		this->mMysqlComponent = this->GetComponent<MysqlDBComponent>();
		ProtoComponent* component = this->GetComponent<ProtoComponent>();
		LOG_CHECK_RET_FALSE(component->Import("mysql/server.proto"));
		std::shared_ptr<Message> message = component->New("server.registry");
		if(message == nullptr)
		{
			LOG_ERROR("create protobuf type [server.registry] error");
			return false;
		}
		this->mTable = message->GetTypeName();
        this->mIndex = this->mMysqlComponent->MakeMysqlClient();
		std::vector<std::string> keys{ "name", "rpc" };
		std::shared_ptr<Mysql::CreateTabCommand> command =
                std::make_shared<Mysql::CreateTabCommand>(message, keys);
        return this->mMysqlComponent->Run(this->mIndex , command)->IsOk();
	}

	int Registry::Query(const com::array::string& request, s2s::server::list& response)
	{
		std::stringstream sqlStream;
		sqlStream << "select (name,rpc,http,time) from " << this->mTable << " where ";
		if (request.array_size() == 0)
		{
			std::vector<std::string> servers;
			this->mInnerComponent->GetServiceList(servers);
			for (size_t index = 0; index < servers.size(); index++)
			{
				sqlStream << "name=" << servers[index];
				if (index < servers.size() - 1)
				{
					sqlStream << " or ";
				}
			}
		}
		else
		{
			for (int index = 0; index < request.array_size(); index++)
			{
				sqlStream << "name=" << request.array(index);
				if (index < request.array_size() - 1)
				{
					sqlStream << " or ";
				}
			}
		}
		const std::string sql = sqlStream.str();
		return XCode::Successful;
	}

	int Registry::Register(const s2s::server::info& request)
	{
		if(request.rpc().empty() || request.name().empty())
		{
			return XCode::CallArgsError;
		}
		const std::string& rpc = request.rpc();
		const std::string& http = request.http();
		const std::string& name = request.name();

        long long time = Helper::Time::NowSecTime();
        const std::string sql = fmt::format("replace into {0} (name,rpc,http,time) values('{1}','{2}','{3}',{4});",
			this->mTable, name, rpc, http, time);

        if(!this->mMysqlComponent->Execute(this->mIndex, std::make_shared<Mysql::SqlCommand>(sql)))
        {
            return XCode::SaveToMysqlFailure;
        }

		const std::string func("Join");
        this->mNodeComponent->AddRpcServer(name, rpc);
        this->mNodeComponent->AddHttpServer(name, http);

		std::vector<std::string> clients;
		RpcService* rpcService = this->mApp->GetService<Node>();
		if(this->mInnerComponent->GetConnectClients(clients) > 0)
		{
			for (const std::string& address : clients)
			{
				rpcService->Send(address, func, request);
			}
		}
		return XCode::Successful;
	}

	void Registry::OnNodeServerError(const std::string& address)
	{

	}

	int Registry::Ping(const Rpc::Packet& packet)
	{
		const std::string & address = packet.From();
		const ServiceNodeInfo * nodeInfo = this->mInnerComponent->GetSeverInfo(address);
		if(nodeInfo != nullptr)
		{
            long long time = Helper::Time::NowSecTime();
			const std::string table("server.registry");
			const std::string & rpc = nodeInfo->RpcAddress;
            const std::string sql = fmt::format("update {0} set time={1} where rpc='{2}'", this->mTable, time, rpc);
            if(!this->mMysqlComponent->Execute(this->mIndex, std::make_shared<Mysql::SqlCommand>(sql)))
            {
                return XCode::SaveToMysqlFailure;
            }
		}
		return XCode::Successful;
	}

	int Registry::UnRegister(const com::type::string& request)
	{
		const std::string & rpc = request.str();
		if(!this->mNodeComponent->DelServer(rpc))
		{
			return XCode::Failure;
		}
		RpcService* rpcService = this->mApp->GetService<Node>();
        const std::string sql = fmt::format("delete from {0} where rpc='{1}'", this->mTable, rpc);
        if(!this->mMysqlComponent->Run(this->mIndex, std::make_shared<Mysql::SqlCommand>(sql))->IsOk())
        {
            return XCode::SaveToMysqlFailure;
        }
		const std::string func("Exit");
		std::vector<std::string> clients;
		if(this->mInnerComponent->GetConnectClients(clients) > 0)
		{
			for(const std::string & address : clients)
			{
				rpcService->Send(address, func, request);
			}
		}
		return XCode::Successful;
	}

	void Registry::OnSecondUpdate(int tick)
	{
		if (tick % 10 != 0) return;
		long long now = Helper::Time::NowSecTime();
	}
}