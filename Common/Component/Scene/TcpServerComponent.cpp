#include "TcpServerComponent.h"

#include<Core/App.h>
#include<Util/StringHelper.h>
#include<NetWork/TcpClientSession.h>
#include<Scene/TcpNetSessionComponent.h>
#include<Scene/TaskPoolComponent.h>
#include <NetWork/NetworkListener.h>
#include <Thread/TaskThread.h>
namespace Sentry
{
    bool TcpServerComponent::Awake()
    {
		ServerConfig & config = App::Get().GetConfig();
		config.GetValue("WhiteList", this->mWhiteList);
		TaskPoolComponent * taskComponent = this->GetComponent<TaskPoolComponent>();
		rapidjson::Value * jsonValue = config.GetJsonValue("Listener");
		if (jsonValue == nullptr || !jsonValue->IsObject())
		{
			return false;
		}
		for (auto iter = jsonValue->MemberBegin(); iter != jsonValue->MemberEnd(); iter++)
		{
			if (!iter->value.IsObject())
			{
				continue;
			}

            ListenConfig listenConfig;
            listenConfig.Name = iter->name.GetString();
            listenConfig.Port = iter->value["port"].GetUint();
            listenConfig.Count = iter->value["count"].GetInt();
            listenConfig.Handler = iter->value["handler"].GetString();;

			if (!dynamic_cast<ISocketHandler*>(this->gameObject->GetComponentByName(listenConfig.Handler)))
			{
				return false;
			}
			NetWorkThread * netThread = taskComponent->NewNetworkThread(listenConfig.Name, nullptr);
			if (netThread == nullptr)
			{
				return false;
			}
			auto listener = new NetworkListener(netThread, listenConfig);
			this->mListenerMap.insert(std::make_pair(listenConfig.Name, listener));

		}
		return true;
    }

    void TcpServerComponent::Start()
    {
		auto iter = this->mListenerMap.begin();
		for (; iter != this->mListenerMap.end(); iter++)
		{
            const ListenConfig & config = iter->second->GetConfig();
			Component * component = this->gameObject->GetComponentByName(config.Handler);
            if(auto handler = dynamic_cast<ISocketHandler*>(component))
            {
                iter->second->StartListen(handler);
            }
		}
    }
}
