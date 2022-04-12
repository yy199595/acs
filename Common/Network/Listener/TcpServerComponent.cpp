#include"TcpServerComponent.h"
#include"App/App.h"
#include<Thread/TaskThread.h>
#include<Util/StringHelper.h>
#include<Component/Scene/ThreadPoolComponent.h>
#include<Network/Listener/NetworkListener.h>
namespace Sentry
{
	bool TcpServerComponent::LoadServerConfig()
	{
		const ServerConfig& config = App::Get()->GetConfig();
		config.GetMember("listener", "ip", this->mHostIp);
		const rapidjson::Value* jsonValue = config.GetJsonValue("listener");
		if (jsonValue == nullptr || !jsonValue->IsObject())
		{
			return false;
		}
		std::unordered_map<std::string, const rapidjson::Value*> listeners;
		config.GetMember("listener", listeners);
		for (auto iter = listeners.begin(); iter != listeners.end(); iter++)
		{
			const rapidjson::Value& jsonObject = *iter->second;
			if (jsonObject.IsObject())
			{
				ListenConfig* listenConfig = new ListenConfig();
				listenConfig->Port = 0;
				listenConfig->Ip = this->mHostIp;
				if (jsonObject.HasMember("port"))
				{
					listenConfig->Port = jsonObject["port"].GetUint();
					listenConfig->Count = jsonObject["count"].GetInt();
				}
				listenConfig->Name = iter->first;
				listenConfig->Handler = jsonObject["component"].GetString();;
				listenConfig->mAddress = fmt::format("{0}:{1}", listenConfig->Ip, listenConfig->Port);
				this->mListenerConfigs.emplace_back(listenConfig);
			}
		}
		return true;
	}

	const ListenConfig* TcpServerComponent::GetTcpConfig(const std::string& name)
	{
		for (ListenConfig * listenerConfig : this->mListenerConfigs)
		{
			if (listenerConfig->Name == name)
			{
				return listenerConfig;
			}
		}
		return nullptr;
	}

	void TcpServerComponent::GetListenConfigs(std::vector<const ListenConfig*>& configs)
	{
		configs.clear();
		for (ListenConfig * listenerConfig : this->mListenerConfigs)
		{
			configs.emplace_back(listenerConfig);
		}
	}

	bool TcpServerComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->LoadServerConfig());
		ThreadPoolComponent * taskComponent = this->GetComponent<ThreadPoolComponent>();
		for (auto listenConfig : this->mListenerConfigs)
		{
			ISocketListen* socketHandler = this->GetComponent<ISocketListen>(listenConfig->Handler);
			if (socketHandler == nullptr)
			{
				LOG_ERROR("not find socket handler " << listenConfig->Handler);
				return false;
			}
#ifdef ONLY_MAIN_THREAD
			IAsioThread& netThread = App::Get()->GetTaskScheduler();
#else
			IAsioThread &netThread = taskComponent->AllocateNetThread();
#endif
			if (listenConfig->Port != 0)
			{
				auto listener = new NetworkListener(netThread, *listenConfig);
				this->mListeners.push_back(listener);
			}
		}
		return true;
	}

	void TcpServerComponent::OnAllServiceStart()
	{
		for (NetworkListener * listener : this->mListeners)
		{
			const ListenConfig& config = listener->GetConfig();
			ISocketListen* listenerHandler = this->GetComponent<ISocketListen>(config.Handler);
			if(listenerHandler != nullptr)
			{
				if (listener->StartListen(listenerHandler))
				{
					const ListenConfig& config = listener->GetConfig();
					LOG_DEBUG(config.Name << " listen " << config.Ip << ":" << config.Port << " successful");
				}
			}
		}
	}
	std::string TcpServerComponent::GetTcpAddress(const string& name)
	{
		const ListenConfig * listenConfig = this->GetTcpConfig(name);
		if(listenConfig == nullptr)
		{
			return std::string();
		}
		return fmt::format("{0}:{1}", listenConfig->Ip, listenConfig->Port);
	}

	bool TcpServerComponent::StartListen(const string& name)
	{
		for (NetworkListener * listener : this->mListeners)
		{
			const ListenConfig& config = listener->GetConfig();
			ISocketListen* listenerHandler = this->GetComponent<ISocketListen>(config.Handler);
			if(listenerHandler != nullptr && config.Name == name)
			{
				if (listener->StartListen(listenerHandler))
				{
					const ListenConfig& config = listener->GetConfig();
					LOG_DEBUG(config.Name << " listen " << config.Ip << ":" << config.Port << " successful");
					return true;
				}
			}
		}
		return false;
	}
}
