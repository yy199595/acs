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
namespace Tendo
{
	Registry::Registry()
	{
		this->mRedisLuaComponent = nullptr;
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
		BIND_COMMON_RPC_METHOD(Registry::UnRegister);
		this->mRedisLuaComponent = this->GetComponent<RedisLuaComponent>();
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

	int Registry::Query(const com::type::string& request, s2s::server::list& response)
	{
		const std::string func("registry.query");
		LOG_ERROR_RETURN_CODE(!request.str().empty(), XCode::CallArgsError);

		Json::Writer jsonWriter;
		jsonWriter.Add("name").Add(request.str());
		std::shared_ptr<RedisResponse> response1 =
				this->mRedisLuaComponent->Call(func, jsonWriter.JsonString(), false);
		LOG_ERROR_RETURN_CODE(response1 && response1->GetType() == RedisRespType::REDIS_ARRAY, XCode::Failure);
		for(size_t index = 0; index < response1->GetArraySize(); index++)
		{
			const RedisString * redisString = response1->Get(index)->Cast<RedisString>();
			if(redisString != nullptr && !redisString->GetValue().empty())
			{
				s2s::server::info * info = response.add_list();
				if(!util::JsonStringToMessage(redisString->GetValue(), info).ok())
				{
					return XCode::JsonCastProtoFailure;
				}
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
		std::string json;
		const std::string func("registry.add");
		LOG_ERROR_RETURN_CODE(!request.name().empty(), XCode::CallArgsError);
		LOG_ERROR_RETURN_CODE(!request.listens().empty(), XCode::CallArgsError);
		if(!util::MessageToJsonString(request, &json).ok())
		{
			return XCode::ProtoCastJsonFailure;
		}
		std::shared_ptr<RedisResponse> response =
				this->mRedisLuaComponent->Call(func, json, false);
		LOG_ERROR_RETURN_CODE(response && response->GetNumber() == 0, XCode::SaveToRedisFailure);

		this->mServers.emplace(address, request.name());
		RpcService * node = this->mApp->GetService<Node>();
		for(auto iter = this->mServers.begin(); iter != this->mServers.end(); iter++)
		{
			node->Send(iter->first, "Join", request);
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
		const std::string & name = iter->second;
		LOG_WARN(name << " [" << address << "] disconnect ");
	}
}