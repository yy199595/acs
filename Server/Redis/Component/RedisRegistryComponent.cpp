#include"RedisRegistryComponent.h"
#include"App/App.h"
#include"Service/LocalService.h"
#include"Listener/TcpListenerComponent.h"
#include"Component/RedisDataComponent.h"
#include"Component/RedisSubComponent.h"

namespace Sentry
{
    bool ServiceNode::RemoveService(const std::string &service)
    {
        auto iter = this->mServices.find(service);
        if(iter != this->mServices.end())
        {
            this->mServices.erase(iter);
            return true;
        }
        return false;
    }
}

namespace Sentry
{
	bool RedisRegistryComponent::LateAwake()
	{
        const ServerConfig * config = ServerConfig::Get();
        this->mTaskComponent = this->GetComponent<TaskComponent>();
        this->mRedisComponent = this->GetComponent<RedisDataComponent>();
		LOG_CHECK_RET_FALSE(config->GetMember("area_id", this->mAreaId));
        LOG_CHECK_RET_FALSE(config->GetLocation("rpc", this->mRpcAddress));
        return true;
	}

	bool RedisRegistryComponent::OnRegisterEvent(NetEventRegistry& eventRegister)
	{
        eventRegister.Sub("Ping", &RedisRegistryComponent::Ping, this);
        eventRegister.Sub("AddNode", &RedisRegistryComponent::AddNode, this);
		eventRegister.Sub("AddService", &RedisRegistryComponent::AddService, this);
        eventRegister.Sub("RemoveService", &RedisRegistryComponent::RemoveService, this);
        return true;
	}

    bool RedisRegistryComponent::Ping(const Json::Reader &json)
    {
        std::string address;
        LOG_CHECK_RET_FALSE(json.GetMember("address", address));
        auto iter = this->mNodes.find(address);
        if(iter == this->mNodes.end())
        {
            LOG_ERROR("not find node address = " << address);
            return false;
        }
        iter->second->UpdateTime();
        return true;
    }

    void RedisRegistryComponent::OnSecondUpdate(const int tick)
    {
        if(tick % 10 == 0)
        {
           TcpRedisClient * redisClient = this->mRedisComponent->GetClient("main");
           if(redisClient != nullptr)
           {
               Json::Writer json;
               json << "address" << this->mRpcAddress;
               std::shared_ptr<RedisRequest> request = RedisRequest::Make(
                       "PUBLISH", "RedisRegistryComponent.Ping", json.JsonString());
               redisClient->SendCommand(request);
           }
           auto iter = this->mNodes.begin();
           long long nowTime = Helper::Time::GetNowSecTime();
           for(; iter != this->mNodes.end(); iter++)
           {
               ServiceNode * node = iter->second;
			   if(node->GetHost() != this->mRpcAddress)
			   {
				   if (nowTime - node->GetLastTime() >= 20)
				   {
					   LOG_ERROR(node->GetHost() << " time out");
				   }
			   }
           }
        }
    }

	void RedisRegistryComponent::OnAddService(const std::string & name)
    {
        Json::Writer jsonWriter;
        jsonWriter << "address" << this->mRpcAddress << "service" << name;
        const std::string message = jsonWriter.JsonString();
        const std::string channel("RedisRegistryComponent.AddService");
        this->mRedisComponent->SenCommond("main", "PUBLISH", channel, message);
        this->mRedisComponent->SenCommond("main", "SADD", this->mRpcAddress, name);
    }

	void RedisRegistryComponent::OnDelService(const std::string & name)
    {
        std::string message;
        Json::Document document;
        document.Add("service", name);
        document.Add("address", this->mRpcAddress);

        document.Serialize(&message);
        const std::string chanel("RedisRegistryComponent.RemoveService");
        this->mRedisComponent->SenCommond("main", "PUBLISH", chanel, message);
        this->mRedisComponent->SenCommond("main", "SREM", this->mRpcAddress, name);
    }

    bool RedisRegistryComponent::RemoveService(const Json::Reader &json)
    {
        std::string address, service;
        LOG_CHECK_RET_FALSE(json.GetMember("address", address));
        LOG_CHECK_RET_FALSE(json.GetMember("service", service));
        Service * component = this->GetApp()->GetService(service);
        if(component != nullptr)
        {
            component->DelLocation(address);
        }
        auto iter = this->mNodes.find(address);
        if(iter != this->mNodes.end())
        {
            iter->second->RemoveService(service);
        }
        return true;
    }

	void RedisRegistryComponent::OnComplete()//通知其他服务器 我加入了
    {
        std::shared_ptr<RedisResponse> response1 =
                this->mRedisComponent->RunCommand("main", "SMEMBERS", this->mRpcAddress);
        if(response1 != nullptr && response1->GetArraySize() > 0)
        {
            std::shared_ptr<RedisRequest> redisRequest(new RedisRequest("SREM"));

            redisRequest->AddParameter(this->mRpcAddress);
            for(size_t index = 0; index < response1->GetArraySize(); index++)
            {
                const RedisString * redisString = response1->Get(index)->Cast<RedisString>();
                if(redisString != nullptr)
                {
                    redisRequest->AddParameter(redisString->GetValue());
                }
            }
            response1 = this->mRedisComponent->Run("main", redisRequest);
            LOG_INFO("remove number = " << response1->GetNumber());
        }
        Json::Writer json;
        std::shared_ptr<RedisRequest> redisRequest(new RedisRequest("SADD"));

        json.BeginArray("services");
        std::vector<Service *> components;
        redisRequest->AddParameter(this->mRpcAddress);
        this->GetApp()->GetServices(components);
        for (Service *component: components)
        {
            if (component->IsStartService())
            {
                json << component->GetName();
                redisRequest->AddParameter(component->GetName());
            }
        }

        this->mRedisComponent->Run("main", redisRequest);

        json << Json::End::EndArray;
        json << "address" << this->mRpcAddress;
        const std::string message = json.JsonString();
        const std::string channel = "RedisRegistryComponent.AddNode";
        std::shared_ptr<RedisResponse> response2 =
                this->mRedisComponent->RunCommand("main", "PUBLISH", channel, message);
        if(response2 != nullptr && response2->GetNumber() > 0)
        {
            LOG_INFO("publish channel number = " << response2->GetNumber());
        }
        this->QueryAllNodes();
    }

    void RedisRegistryComponent::QueryAllNodes()
    {
        std::shared_ptr<RedisResponse> response =
            this->mRedisComponent->RunCommand("main", "PUBSUB", "CHANNELS", "*:*");
        if (response == nullptr || response->GetArraySize() == 0)
        {
            return;
        }
        for (size_t index = 0; index < response->GetArraySize(); index++)
        {
            const RedisString *redisString = response->Get(index)->Cast<RedisString>();
            ServiceNode *serviceNode = new ServiceNode(redisString->GetValue());
            std::shared_ptr<RedisResponse> response1 =
                this->mRedisComponent->RunCommand("main", "SMEMBERS", redisString->GetValue());
            for (size_t x = 0; x < response1->GetArraySize(); x++)
            {
                const RedisString *redisString2 = response1->Get(x)->Cast<RedisString>();
                Service *service = this->GetApp()->GetService(redisString2->GetValue());
                if (service != nullptr)
                {
                    service->AddLocation(redisString->GetValue());
                    serviceNode->AddService(redisString2->GetValue());
                }
            }
            this->mNodes.emplace(serviceNode->GetHost(), serviceNode);
        }
    }

	bool RedisRegistryComponent::AddNode(const Json::Reader &json)
	{
        std::string address;
		std::vector<std::string> services;
        LOG_CHECK_RET_FALSE(json.GetMember("address", address));
        LOG_CHECK_RET_FALSE(json.GetMember("services", services));
        auto iter = this->mNodes.find(address);
        if(iter == this->mNodes.end())
        {
            ServiceNode * node = new ServiceNode(address);
            this->mNodes.emplace(address, node);
        }
        ServiceNode * node = this->mNodes[address];
		for(const std::string & service : services)
		{
			Service * localRpcService = this->GetApp()->GetService(service);
			if(localRpcService != nullptr)
			{
                localRpcService->AddLocation(address);
			}
            node->AddService(service);
		}
		return true;
	}

	bool RedisRegistryComponent::AddService(const Json::Reader &json)
	{
		std::string address, service;
		LOG_CHECK_RET_FALSE(json.GetMember("address", address));
		LOG_CHECK_RET_FALSE(json.GetMember("service", service));
        auto iter = this->mNodes.find(address);
        assert(iter != this->mNodes.end());
        ServiceNode * node = iter->second;
		Service * localRpcService = this->GetApp()->GetService(service);
		if(localRpcService != nullptr)
		{
            node->AddService(service);
            localRpcService->AddLocation(address);
			return true;
		}
		return false;
	}

	void RedisRegistryComponent::OnDestory()
	{
		std::vector<Service*> compontns;
		this->GetApp()->GetServices(compontns);
		for(Service * component : compontns)
		{
			if(component->IsStartService())
			{
				Json::Writer jsonWriter;
                std::shared_ptr<Json::Reader> response(new Json::Reader());
                jsonWriter << "address" << this->mRpcAddress << "service" << component->GetName();
				this->mRedisComponent->Call("main", "node.del", jsonWriter, response);
			}
		}
	}
}// namespace Sentry