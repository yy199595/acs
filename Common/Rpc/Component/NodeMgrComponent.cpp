//
// Created by zmhy0073 on 2022/8/12.
//

#include"NodeMgrComponent.h"
#include"Log/CommonLogDef.h"
#include"Config/ServerConfig.h"
#include"Service/Registry.h"
#include"Math/MathHelper.h"
#include"Config/ClusterConfig.h"
#include"Component/TaskComponent.h"
namespace Sentry
{
    void NodeMgrComponent::AddRpcServer(const std::string& name, const std::string& address)
    {
		const NodeConfig* config = ClusterConfig::Inst()->GetConfig(name);
        if(address.empty() || config == nullptr)
        {
            return;
        }
		auto iter = this->mRpcServers.find(name);
		if(iter == this->mRpcServers.end())
		{
			std::vector<std::string> item;
			this->mRpcServers.emplace(name, item);
		}
        std::vector<std::string> & locations = this->mRpcServers[name];
        if(std::find(locations.begin(), locations.end(), address) == locations.end())
        {
            locations.emplace_back(address);
            LOG_WARN(name << " add rpc server address [" << address << "]");
        }
		if (config->HasService(ComponentFactory::GetName<Registry>()))
		{
			if (std::find(this->mRegistryAddress.begin(),
				this->mRegistryAddress.end(), address) == this->mRegistryAddress.end())
			{
				this->mRegistryAddress.push_back(address);
			}
		}
    }

	void NodeMgrComponent::WaitServerStart(const std::string& server)
	{
		int count = 0;
		TaskComponent * taskComponent = this->GetComponent<TaskComponent>();
		auto iter = this->mRpcServers.find(server);
		while(iter == this->mRpcServers.end() || iter->second.empty())
		{
			count++;
			taskComponent->Sleep(2000);
			iter = this->mRpcServers.find(server);
			LOG_WARN("wait " << server << " start count = " << count);
		}
	}

	void NodeMgrComponent::AddHttpServer(const std::string& name, const std::string& address)
	{
		if(address.empty())
		{
			return;
		}
		LOG_WARN(name << " add http server address [" << address << "]");
	}

	bool NodeMgrComponent::DelServer(const std::string& address)
	{
		auto iter = this->mRpcServers.begin();
		for (; iter != this->mRpcServers.end(); iter++)
		{
			auto iter1 = std::find(
				iter->second.begin(), iter->second.end(), address);
			if (iter1 != iter->second.end())
			{
				iter->second.erase(iter1);
				return true;
			}
		}
		return false;
	}

	bool NodeMgrComponent::GetServers(std::vector<std::string>& hosts)
	{
		auto iter = this->mRpcServers.begin();
		for(; iter != this->mRpcServers.end(); iter++)
		{
			for(const std::string & address : iter->second)
			{
				hosts.emplace_back(address);
			}
		}
		return !hosts.empty();
	}

	bool NodeMgrComponent::GetServers(const std::string& server, std::vector<std::string>& hosts)
	{
		auto iter = this->mRpcServers.find(server);
		if(iter == this->mRpcServers.end())
		{
			return false;
		}
		hosts = iter->second;
		return true;
	}

    bool NodeMgrComponent::GetServer(const std::string & name, std::string & address)
    {
        auto iter = this->mRpcServers.find(name);
        if(iter == this->mRpcServers.end())
        {
            return false;
        }
        int size = (int)iter->second.size();
        int index = Helper::Math::Random<int>(0, size - 1);
        address = iter->second[index];
        return true;
    }

    bool NodeMgrComponent::GetServer(const std::string & name, long long index, std::string & address)
    {
        auto iter = this->mRpcServers.find(name);
        if(iter == this->mRpcServers.end())
        {
            return false;
        }
        int size = (int)iter->second.size();
        address = iter->second[index % size];
        return true;
    }

	LocationUnit* NodeMgrComponent::GetUnit(long long id) const
	{
		auto iter = this->mUnitLocations.find(id);
		return iter != this->mUnitLocations.end() ? iter->second.get() : nullptr;
	}

	bool NodeMgrComponent::AllotServer(const string& server, string& address)
	{
		auto iter = this->mRpcServers.find(server);
		if(iter == this->mRpcServers.end() || iter->second.empty())
		{
			return false;
		}
		int index = Helper::Math::Random<int>(0, iter->second.size());
		address = iter->second.at(index);
		return true;
	}

	bool NodeMgrComponent::DelUnit(long long id)
	{
		auto iter = this->mUnitLocations.find(id);
		if(iter == this->mUnitLocations.end())
		{
			return false;
		}
		this->mUnitLocations.erase(iter);
		return true;
	}

	bool NodeMgrComponent::AddUnit(std::unique_ptr<LocationUnit> locationUnit)
	{
		if(locationUnit == nullptr)
		{
			return false;
		}
		long long id = locationUnit->GetUnitId();
		auto iter = this->mUnitLocations.find(id);
		if(iter != this->mUnitLocations.end())
		{
			return iter->second.get();
		}
		this->mUnitLocations.emplace(id, std::move(locationUnit));
		return true;
	}

	bool NodeMgrComponent::LateAwake()
	{
		const ServerConfig * config = ServerConfig::Inst();
		LOG_CHECK_RET_FALSE(config->GetMember("registry", this->mRegistryAddress));
		return true;
	}

	void NodeMgrComponent::OnLocalComplete()
	{
		const ServerConfig * config = ServerConfig::Inst();
		RpcService * rpcService = this->mApp->GetService<Registry>();
		for(const std::string & address : this->mRegistryAddress)
		{
			s2s::server::info message;
			message.set_name(config->Name());
			config->GetLocation("rpc", *message.mutable_rpc());
			config->GetLocation("http", *message.mutable_http());
			std::shared_ptr<s2s::server::list> response = std::make_shared<s2s::server::list>();
			if(rpcService->Call(address,"Register", message, response) != XCode::Successful)
			{
				LOG_ERROR("register to [" << address << "] error");
				continue;
			}
			for(int index = 0; index < response->list_size(); index++)
			{
				const s2s::server::info & info = response->list(index);
				{
					this->AddRpcServer(info.name(), info.rpc());
					this->AddHttpServer(info.name(), info.http());
				}
			}
		}		
	}
	void NodeMgrComponent::PingRegistryServer()
	{
		const std::string func("Ping");
		RpcService* rpcService = this->mApp->GetService<Registry>();
		TaskComponent* taskComponent = this->GetComponent<TaskComponent>();
		while (!this->mRegistryAddress.empty())
		{
			taskComponent->Sleep(15 * 1000);
			for (const std::string& address : this->mRegistryAddress)
			{
				if (rpcService->Call(address, func) == XCode::Successful)
				{
					CONSOLE_LOG_INFO("ping registry server [" << address << "] successful");
				}
			}
		}
	}
}