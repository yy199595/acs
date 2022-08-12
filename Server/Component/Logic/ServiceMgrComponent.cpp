#include"ServiceMgrComponent.h"
#include"App/App.h"
#include"Network/Listener/TcpServerListener.h"
#include"Network/Listener/TcpServerComponent.h"
#include"Component/Redis/MainRedisComponent.h"
#include"Component/RpcService/LocalService.h"

namespace Sentry
{
	bool ServiceMgrComponent::LateAwake()
	{
        this->mTaskComponent = this->GetComponent<TaskComponent>();
        this->mRedisComponent = this->GetComponent<MainRedisComponent>();
		LOG_CHECK_RET_FALSE(this->GetConfig().GetMember("area_id", this->mAreaId));
		LOG_CHECK_RET_FALSE(this->GetConfig().GetMember("node_name", this->mNodeName));
		LOG_CHECK_RET_FALSE(this->GetConfig().GetListener("rpc", this->mRpcAddress));
		return true;
	}

	bool ServiceMgrComponent::OnRegisterEvent(NetEventRegistry& eventRegister)
	{
		eventRegister.Sub("OnServiceAdd", &ServiceMgrComponent::OnServiceAdd, this);
		eventRegister.Sub("OnServiceDel", &ServiceMgrComponent::OnServiceDel, this);
		eventRegister.Sub("OnNodeRegister", &ServiceMgrComponent::OnNodeRegister, this);
		return true;
	}

	void ServiceMgrComponent::OnAddService(Component* component)
	{
		if(component->Cast<Service>())
		{
			Json::Writer jsonWriter;
            std::shared_ptr<Json::Reader> response(new Json::Reader());
			jsonWriter << "address" << this->mRpcAddress << "service" << component->GetName();
			if(!this->mRedisComponent->Call("main", "node.add", jsonWriter, response))
			{
				LOG_ERROR("call node.add failure " << component->GetName());
			}
		}
	}

	void ServiceMgrComponent::OnDelService(Component* component)
	{

	}

	void ServiceMgrComponent::OnComplete()//通知其他服务器 我加入了
	{
        if(!this->RegisterService())
        {
            LOG_DEBUG("register this failure");
            return;
        }
        this->mTaskComponent->Start(&ServiceMgrComponent::Update, this);
	}

    void ServiceMgrComponent::Update()
    {
        std::shared_ptr<Json::Writer> json = this->GetServiceJson(true);
        while(true)
        {
            this->mTaskComponent->Sleep(10 * 1000);
            std::shared_ptr<Json::Reader> response(new Json::Reader());
            if (this->mRedisComponent->Call("main", "node.update", *json, response))
            {
                CONSOLE_LOG_INFO("start update this service");
            }
            auto iter = this->mAddressInfos.begin();
            for(; iter != this->mAddressInfos.end();)
            {
                if(!this->RefreshService(iter->first))
                {
                    this->mAddressInfos.erase(iter++);
                    continue;
                }
                iter++;
            }
        }
    }

    std::shared_ptr<Json::Writer> ServiceMgrComponent::GetServiceJson(bool broacast)
    {
        std::shared_ptr<Json::Writer> json(new Json::Writer());
        json->BeginArray("services");
        std::vector<Service *> components;
        this->GetApp()->GetServices(components);
        for (Service *component: components)
        {
            if (component->IsStartService())
            {
                (*json) << component->GetName();
            }
        }
        (*json) << Json::End::EndArray;
        (*json) << "broacast" << true;
        (*json) << "address" << this->mRpcAddress;
        return json;
    }

    bool ServiceMgrComponent::RegisterService()
    {
        std::shared_ptr<Json::Reader> response(new Json::Reader());
        std::shared_ptr<Json::Writer> json = this->GetServiceJson(true);
        if (!this->mRedisComponent->Call("main", "node.update", *json, response))
        {
            LOG_ERROR("register failure");
            return false;
        }
        const rapidjson::Value *jsonValue = response->GetJsonValue("services");
        if(jsonValue == nullptr || !jsonValue->IsObject())
        {
            return false;
        }
        auto iter = jsonValue->MemberBegin();
        for (; iter != jsonValue->MemberEnd(); iter++)
        {
            std::string address(iter->name.GetString());
            const rapidjson::Value &jsonData = iter->value;
            rapidjson::Type type = jsonData.GetType();
            for (int index = 0; index < jsonData.Size(); index++)
            {
                assert(jsonData[index].IsString());
                std::string service(jsonData[index].GetString());
                Service *component = this->GetApp()->GetService(service);
                if (component != nullptr && !component->HasHost(address))
                {
                    component->AddHost(address);
                    LOG_INFO(service << " add new address [" << address << "]");
                }
            }
        }
        return true;
    }

	void ServiceMgrComponent::RemoveAddress(const std::string & address)
	{
		std::vector<Service *> services;
		this->GetApp()->GetServices(services);
		for(Service * component : services)
		{
			if(component->DelHost(address))
			{
				LOG_ERROR("remove [" << address << "] from " << component->GetName());
			}
		}
	}


	bool ServiceMgrComponent::OnNodeRegister(const Json::Reader& json)
	{
        int second = 0;
        std::string address;
		std::vector<std::string> services;
        LOG_CHECK_RET_FALSE(json.GetMember("t", second));
        LOG_CHECK_RET_FALSE(json.GetMember("address", address));
        LOG_CHECK_RET_FALSE(json.GetMember("services", services));
		for(const std::string & service : services)
		{
			Service * localRpcService = this->GetApp()->GetService(service);
			if(localRpcService != nullptr)
			{
				localRpcService->AddHost(address);
			}
            this->mAddressInfos[address].insert(service);
		}
		return true;
	}

    bool ServiceMgrComponent::RefreshService(const std::string &address)
    {
        std::shared_ptr<Json::Writer> request(new Json::Writer());
        (*request) << "address" << address;
        std::shared_ptr<Json::Reader> response(new Json::Reader());
        if (this->mRedisComponent->Call("main", "node.query", *request, response))
        {
            bool res = false;
            if (response->GetMember("res", res) && res)
            {
                return false;
            }
            auto iter = this->mAddressInfos.find(address);
            if (iter != this->mAddressInfos.end())
            {
                for (const std::string &service: iter->second)
                {
                    Service *component = this->GetApp()->GetService(service);
                    if (component != nullptr && component->HasHost(address))
                    {
                        component->DelHost(address);
                        CONSOLE_LOG_FATAL("remove [" << address << "] from " << service);
                    }
                }
                return false;
            }
        }
        return true;
    }

	bool ServiceMgrComponent::OnServiceAdd(const Json::Reader& json)
	{
		std::string address, service;
		LOG_CHECK_RET_FALSE(json.GetMember("address", address));
		LOG_CHECK_RET_FALSE(json.GetMember("service", service));
		Service * localRpcService = this->GetApp()->GetService(service);
		if(localRpcService != nullptr)
		{
			localRpcService->AddHost(address);
			return true;
		}
		return false;
	}

	bool ServiceMgrComponent::OnServiceDel(const Json::Reader& json)
	{
		std::string address, service;
		LOG_CHECK_RET_FALSE(json.GetMember("address", address));
		LOG_CHECK_RET_FALSE(json.GetMember("service", service));
		Service* localRpcService = this->GetApp()->GetService(service);
		return localRpcService != nullptr && localRpcService->DelHost(address);
	}

	void ServiceMgrComponent::OnDestory()
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