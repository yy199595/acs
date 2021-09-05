#include "SceneProtocolComponent.h"

#include <Core/App.h>
#include <Util/FileHelper.h>
#include <Util/StringHelper.h>
#include <google/protobuf/util/json_util.h>
#include <Service/ServiceBase.h>
#include <NetWork/ServiceMethod.h>
#include <rapidjson/document.h>
namespace Sentry
{

	SceneProtocolComponent::SceneProtocolComponent()
	{

	}

	bool SceneProtocolComponent::Awake()
    {
		rapidjson::Document jsonMapper;
		const std::string & dir = App::Get().GetConfigDir();	
		if (!FileHelper::ReadJsonFile(dir + "rpc.json", jsonMapper))
		{
			SayNoDebugFatal("not find file : rpc.json");
			return false;
		}

		auto iter1 = jsonMapper.MemberBegin();
		for (; iter1 != jsonMapper.MemberEnd(); iter1++)
		{
			const std::string service = iter1->name.GetString();
			rapidjson::Value& jsonValue = iter1->value;
			SayNoAssertRetFalse_F(jsonValue.IsObject());
			SayNoAssertRetFalse_F(jsonValue.HasMember("id"));

			std::vector<ProtocolConfig *> methods;
			auto iter2 = jsonValue.MemberBegin();
			for (; iter2 != jsonValue.MemberEnd(); iter2++)
			{
				if (!iter2->value.IsObject())
				{
					continue;
				}
				std::string request;
				std::string response;
				std::string method = iter2->name.GetString();
				SayNoAssertRetFalse_F(iter2->value.HasMember("id"));
				unsigned short id = (unsigned short)iter2->value["id"].GetUint();
				if (iter2->value.HasMember("request"))
				{
					request = iter2->value["request"].GetString();
					Message * message = this->CreateMessage(request);
					if (message == nullptr)
					{
						SayNoDebugFatal("create " << request << " failure");
						return false;
					}
					this->DestoryMessage(message);
				}
				if (iter2->value.HasMember("response"))
				{
					response = iter2->value["response"].GetString();
					Message * message = this->CreateMessage(response);
					if (message == nullptr)
					{
						SayNoDebugFatal("create " << response << " failure");
						return false;
					}
					this->DestoryMessage(message);
				}
				ProtocolConfig *protocol = new ProtocolConfig(id, service, method, request, response);
				if (protocol != nullptr)
				{
					methods.push_back(protocol);
					std::string name = service + "." + method;
					this->mProtocolMap.insert(std::make_pair(id, protocol));
					this->mProtocolNameMap.insert(std::make_pair(name, protocol));
				}
			}
			this->mServiceMap.emplace(service, methods);
		}
        return true;
    }

	void SceneProtocolComponent::Start()
	{
			
	}


	void SceneProtocolComponent::GetServices(std::vector<std::string> & services)
	{
		auto iter = this->mServiceMap.begin();
		for (; iter != this->mServiceMap.end(); iter++)
		{
			services.push_back(iter->first);
		}
	}

	bool SceneProtocolComponent::GetMethods(const std::string service, std::vector<std::string> & methods)
	{
		auto iter = this->mServiceMap.find(service);
		if (iter == this->mServiceMap.end())
		{
			return false;
		}
		for (ProtocolConfig * config : iter->second)
		{
			methods.push_back(config->MethodName);
		}
		return true;
	}

	const ProtocolConfig *SceneProtocolComponent::GetProtocolConfig(unsigned short id) const
    {
        auto iter = this->mProtocolMap.find(id);
        return iter != this->mProtocolMap.end() ? iter->second : nullptr;
    }

    const ProtocolConfig *
    SceneProtocolComponent::GetProtocolConfig(const std::string &service, const std::string &method) const
    {
        std::string name = service + "." + method;
        auto iter = this->mProtocolNameMap.find(name);
        return iter != this->mProtocolNameMap.end() ? iter->second : nullptr;
    }

    Message *SceneProtocolComponent::CreateMessage(const std::string &name)
    {
        auto iter = this->mProtocolPoolMap.find(name);
        if (iter != this->mProtocolPoolMap.end())
        {
            if (iter->second.size() > 0)
            {
                Message *message = iter->second.front();
                iter->second.pop();
                return message;
            }
        }
        const DescriptorPool *pDescriptorPool = DescriptorPool::generated_pool();
        const Descriptor *pDescriptor = pDescriptorPool->FindMessageTypeByName(name);
        if (pDescriptor == nullptr)
        {
            SayNoDebugError("create pb fail " << name);
            return nullptr;
        }
        MessageFactory *factory = MessageFactory::generated_factory();
        const Message *pMessage = factory->GetPrototype(pDescriptor);

        return pMessage->New();
    }

    Message *SceneProtocolComponent::CreateMessageByJson(const std::string &name, const char *msg, const size_t size)
    {
        Message *message = this->CreateMessage(name);
        if (message != nullptr)
        {
            if (util::JsonStringToMessage(std::string(msg, size), message).ok())
            {
                return message;
            }
            return message;
        }
        return nullptr;
    }

	bool SceneProtocolComponent::GetJsonByMessage(Message * message, std::string & json)
	{
		if (message == nullptr)
		{
			return false;
		}
		return util::MessageToJsonString(*message, &json).ok();
	}

	bool SceneProtocolComponent::DestoryMessage(Message *message)
    {
        if (message == nullptr)
        {
            return true;
        }
        const std::string name = message->GetTypeName();
        std::queue<Message *> &pool = this->mProtocolPoolMap[name];

        if (pool.size() >= 100)
        {
            delete message;
            return true;
        }
        message->Clear();
        pool.push(message);
        return false;
    }

}
