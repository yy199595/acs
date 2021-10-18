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

			const std::string name = iter->name.GetString();
			const int count = iter->value["count"].GetInt();
			unsigned short port = iter->value["port"].GetUint();
			const std::string component = iter->value["handler"].GetString();
			ISocketHandler * socketHandler = dynamic_cast<ISocketHandler*>(this->gameObject->GetComponentByName(component));
			if (socketHandler == nullptr)
			{
				return false;
			}
			NetWorkThread * netThread = taskComponent->NewNetworkThread(name, nullptr);
			if (netThread == nullptr)
			{
				return false;
			}
			NetworkListener * listener = new NetworkListener(name, netThread, port, count);
			this->mListenerMap.insert(std::make_pair(name, listener));

		}
		return true;
    }

    void TcpServerComponent::Start()
    {
		auto iter = this->mListenerMap.begin();
		for (; iter != this->mListenerMap.end(); iter++)
		{			
			const std::string & name = iter->second->GetName();
			Component * component = this->gameObject->GetComponentByName(name);
			iter->second->StartListen(dynamic_cast<ISocketHandler*>(component));
		}
    }
}
