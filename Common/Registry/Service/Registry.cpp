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
		LOG_CHECK_RET_FALSE(component->Import("mysql/registry.proto"));
		std::shared_ptr<Message> message = component->New("registry.server");

        this->mIndex = this->mMysqlComponent->MakeMysqlClient();
		std::vector<std::string> keys{ "name", "rpc" };
		std::shared_ptr<Mysql::CreateTabCommand> command =
                std::make_shared<Mysql::CreateTabCommand>(message, keys);
        return this->mMysqlComponent->Run(this->mIndex , command)->IsOk();
	}

	int Registry::Query(const com::array::string& request, s2s::server::list& response)
	{		
		std::vector<std::string> servers;
		if (request.array_size() == 0)
		{
			this->mInnerComponent->GetServiceList(servers);
		}
		else
		{
			for (int index = 0; index < request.array_size(); index++)
			{
				servers.emplace_back(request.array(index));
			}
		}
		
//		for (const ServiceNodeInfo* nodeInfo : services)
//		{
//			s2s::server::info * info = response.add_list();
//			{
//				info->set_name(nodeInfo->SrvName);
//				info->set_rpc(nodeInfo->RpcAddress);
//				info->set_http(nodeInfo->HttpAddress);
//			}
//		}
		return XCode::Successful;
	}

	int Registry::Register(const s2s::server::info& request, s2s::server::list& response)
	{
		if(request.rpc().empty() || request.name().empty())
		{
			return XCode::CallArgsError;
		}
		const std::string& rpc = request.rpc();
		const std::string& http = request.http();
		const std::string& name = request.name();

        long long time = Helper::Time::NowSecTime();
        const std::string sql = fmt::format("replace into registry.server "
                                            "(name,rpc,http,time) values('{0}','{1}','{2}',{3});", name, rpc, http, time);

        if(!this->mMysqlComponent->Run(this->mIndex, std::make_shared<Mysql::SqlCommand>(sql))->IsOk())
        {
            return XCode::SaveToMysqlFailure;
        }

		const std::string func("Join");
        this->mNodeComponent->AddRpcServer(name, rpc);
        this->mNodeComponent->AddHttpServer(name, http);
        RpcService* rpcService = this->mApp->GetService<Node>();

		std::vector<const ServiceNodeInfo*> services;
		this->mInnerComponent->GetServiceList(services);
		for (const ServiceNodeInfo* nodeInfo : services)
		{
			const std::string& address = nodeInfo->LocalAddress;
			if (rpcService->Call(address, func, request) == XCode::Successful)
			{
				s2s::server::info* server = response.add_list();
				{
					server->set_name(nodeInfo->SrvName);
					server->set_rpc(nodeInfo->RpcAddress);
					server->set_http(nodeInfo->HttpAddress);
				}
			}
		}
		const ServerConfig* config = ServerConfig::Inst();
		s2s::server::info* localServer = response.add_list();
		{
			localServer->set_name(config->Name());
			config->GetLocation("rpc", *localServer->mutable_rpc());
			config->GetLocation("http", *localServer->mutable_http());
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
			const std::string & rpc = nodeInfo->RpcAddress;
            const std::string sql = fmt::format("update registry.server set time={0} where rpc='{1}'", time, rpc);
            if(!this->mMysqlComponent->Run(this->mIndex, std::make_shared<Mysql::SqlCommand>(sql))->IsOk())
            {
                return XCode::SaveToMysqlFailure;
            }
		}
		return XCode::Successful;
	}

	int Registry::UnRegister(const com::type::string& request)
	{
		const std::string & address = request.str();
		if(!this->mNodeComponent->DelServer(address))
		{
			return XCode::Failure;
		}
		std::vector<const ServiceNodeInfo*> services;
		this->mInnerComponent->GetServiceList(services);
		RpcService* rpcService = this->mApp->GetService<Node>();
        const std::string sql = fmt::format("delete from registry.server where rpc='{0}'", address);
        if(!this->mMysqlComponent->Run(this->mIndex, std::make_shared<Mysql::SqlCommand>(sql))->IsOk())
        {
            return XCode::SaveToMysqlFailure;
        }

		for (const ServiceNodeInfo* nodeInfo : services)
		{
			if(nodeInfo->RpcAddress != address)
			{
				const std::string& target = nodeInfo->LocalAddress;
				rpcService->Send(target, "Exit", request);
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