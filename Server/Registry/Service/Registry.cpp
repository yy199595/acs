//
// Created by zmhy0073 on 2022/10/25.
//

#include"Registry.h"
#include"Entity/Unit/App.h"
#include"Common/Service/Node.h"
#include"Message/db.pb.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Rpc/Component/InnerNetComponent.h"
#include"Rpc/Component/NodeMgrComponent.h"

#include"Proto/Component/ProtoComponent.h"
#include"Redis/Component/RedisComponent.h"

namespace Tendo
{
	Registry::Registry()
	{
		this->mNodeComponent = nullptr;
		this->mInnerComponent = nullptr;
		this->mRedisComponent = nullptr;
	}

	bool Registry::Awake()
	{
		this->mApp->AddComponent<RedisComponent>();
		return true;
	}

	bool Registry::OnInit()
	{
		BIND_COMMON_RPC_METHOD(Registry::Ping);
		BIND_COMMON_RPC_METHOD(Registry::Query);
		BIND_ADDRESS_RPC_METHOD(Registry::Register);
		BIND_ADDRESS_RPC_METHOD(Registry::UnRegister);
		this->mRedisComponent = this->GetComponent<RedisComponent>();
		this->mNodeComponent = this->GetComponent<NodeMgrComponent>();
		this->mInnerComponent = this->GetComponent<InnerNetComponent>();
		return true;
	}

	bool Registry::OnStart()
	{
		std::shared_ptr<Message> message;
		ProtoComponent* component = this->GetComponent<ProtoComponent>();
		LOG_CHECK_RET_FALSE(component->Import("mysql/server.proto"));
		if (!component->New("server.registry", message))
		{
			LOG_ERROR("create protobuf type [server.registry] error");
			return false;
		}

		return true;
	}

	int Registry::Query(const s2s::server::query& request, s2s::server::list& response)
	{
		std::vector<std::string> list;
		if (!request.server_name().empty())
		{
			const std::string& name = request.server_name();
			if (ClusterConfig::Inst()->GetConfig(name) == nullptr)
			{
				return XCode::NotFoundRpcConfig;
			}
			list.emplace_back(name);
		}
		else
		{		
			ClusterConfig::Inst()->GetServers(list);		
		}
		for (const std::string& server : list)
		{
			std::shared_ptr<RedisResponse> result =
				this->mRedisComponent->Run("HVALS", server);
			for (size_t index = 0; index < result->GetArraySize(); index++)
			{
				const RedisAny* redisAny = result->Get(index);
				if (!redisAny->IsString())
				{
					continue;
				}
				const RedisString* redisString = redisAny->Cast<RedisString>();
				const std::string& json = redisString->GetValue();
				Json::Reader document;
				CONSOLE_LOG_INFO(json);
				if (!document.ParseJson(json))
				{
					return XCode::ParseJsonFailure;
				}
				long long time = 0;
				//long long now = Helper::Time::NowSecTime();
				document.GetMember("time", time);
				//if (now - time <= 30)
				{
					s2s::server::info* message = response.add_list();
					document.GetMember("rpc", *message->mutable_rpc());
					document.GetMember("http", *message->mutable_http());
					document.GetMember("name", *message->mutable_name());
				}
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

		Json::Writer jsonWriter;
		jsonWriter.Add("rpc").Add(rpc);
		jsonWriter.Add("http").Add(http);
		jsonWriter.Add("name").Add(server);
		jsonWriter.Add("time").Add(Helper::Time::NowSecTime());
		{
			std::string value;
			jsonWriter.WriterStream(&value);
			std::shared_ptr<RedisResponse> response = 
				this->mRedisComponent->SyncRun("HSET", server, rpc, value);
			if (response->GetNumber() != 1 && response->GetNumber() != 0)
			{
				return XCode::Failure;
			}
		}

		const std::string func("Join");
		this->mRegistryServers.insert(address);
		this->mNodeComponent->AddRpcServer(server, rpc);
		this->mNodeComponent->AddHttpServer(server, http);
		RpcService* rpcService = this->mApp->GetService<Node>();
		for (const std::string& server: this->mRegistryServers)
		{
			rpcService->Send(server, func, request);
			LOG_INFO("send server join message to " << server);
		}
		return XCode::Successful;
	}

	int Registry::Ping(const Rpc::Packet& packet)
	{
		
		return XCode::Successful;
	}

	int Registry::UnRegister(const std::string& address, const s2s::server::info& request)
	{
		const std::string& rpc = request.rpc();
		const std::string& server = request.name();
		LOG_ERROR_RETURN_CODE(!rpc.empty(), XCode::CallArgsError);
		LOG_ERROR_RETURN_CODE(!server.empty(), XCode::CallArgsError);

		std::shared_ptr<RedisResponse> response =
			this->mRedisComponent->SyncRun("HDEL", server, rpc);
		if (response == nullptr || (response->GetNumber() != 1 && response->GetNumber() != 0))
		{
			return XCode::Failure;
		}
		auto iter = this->mRegistryServers.find(address);
		if (iter != this->mRegistryServers.end())
		{
			this->mRegistryServers.erase(iter);
		}
		LOG_WARN("remove server : " << rpc);
		return XCode::Successful;
	}

	void Registry::OnSecondUpdate(int tick)
	{

	}
	void Registry::OnClose()
	{
		AsyncMgrComponent * component = this->GetComponent<AsyncMgrComponent>();
		for(int index = 0; index < 10; index++)
		{
			component->Sleep(1000);
			CONSOLE_LOG_DEBUG("wait other server close");
		}
	}
}