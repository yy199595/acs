#include"TcpServerComponent.h"
#include"Object/App.h"
#include<Util/StringHelper.h>
#include<Scene/ThreadPoolComponent.h>
#include<Thread/TaskThread.h>
#include<Listener/NetworkListener.h>
namespace Sentry
{
    bool TcpServerComponent::Awake()
    {
		const ServerConfig & config = App::Get().GetConfig();
		config.GetValue("white_list", this->mWhiteList);
        config.GetValue("listener","ip", this->mHostIp);
		rapidjson::Value * jsonValue = config.GetJsonValue("listener");
		if (jsonValue == nullptr || !jsonValue->IsObject())
		{
			return false;
		}
        auto iter = jsonValue->MemberBegin();
		for ( ;iter != jsonValue->MemberEnd(); iter++)
		{
			if (!iter->value.IsObject())
			{
				continue;
			}
            ListenConfig * listenConfig = new ListenConfig();
            listenConfig->Port = 0;
            listenConfig->Ip = this->mHostIp;
            if(iter->value.HasMember("port"))
            {
                listenConfig->Port = iter->value["port"].GetUint();
                listenConfig->Count = iter->value["count"].GetInt();
            }
            listenConfig->Name = iter->name.GetString();
            listenConfig->Handler = iter->value["component"].GetString();;
            listenConfig->mAddress = fmt::format("{0}:{1}", listenConfig->Ip, listenConfig->Port);
            this->mListenerConfigs.emplace_back(listenConfig);
		}
		return true;
    }

    const NetworkListener *TcpServerComponent::GetListener(const std::string &name)
    {
        for(auto listener : this->mListeners)
        {
            if(listener->GetConfig().Name == name)
            {
                return listener;
            }
        }
        return nullptr;
    }

    void TcpServerComponent::GetListeners(std::vector<const NetworkListener *> &listeners)
    {
        listeners.clear();
        for(auto listener : this->mListeners)
        {
            listeners.emplace_back(listener);
        }
    }

    bool TcpServerComponent::LateAwake()
    {
        auto taskComponent = this->GetComponent<ThreadPoolComponent>();
        for(auto listenConfig : this->mListenerConfigs)
        {
            Component *component = this->GetComponent<Component>(listenConfig->Handler);
            auto socketHandler = dynamic_cast<ISocketListen *>(component);
            if (socketHandler == nullptr)
            {
                LOG_ERROR("not find socket handler ", listenConfig->Handler);
                return false;
            }
#ifdef ONLY_MAIN_THREAD
            IAsioThread &netThread = App::Get().GetTaskScheduler();
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

    void TcpServerComponent::OnStart()
    {
        for (auto listener : this->mListeners)
        {
            const ListenConfig &config = listener->GetConfig();
            Component *component = this->GetComponent<Component>(config.Handler);
            if (auto handler = dynamic_cast<ISocketListen *>(component))
            {
                if (listener->StartListen(handler))
                {
                    const ListenConfig &config = listener->GetConfig();
					LOG_DEBUG("{0} listen {1}:{2} successful", config.Name, config.Ip, config.Port);
                }
            }
        }
    }
}
