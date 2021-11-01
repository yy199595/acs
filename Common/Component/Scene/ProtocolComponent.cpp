#include "ProtocolComponent.h"

#include <Core/App.h>
#include <Util/FileHelper.h>
#include <Util/StringHelper.h>
#include <google/protobuf/util/json_util.h>
#include <rapidjson/document.h>
#include <Pool/MessagePool.h>
namespace GameKeeper
{
	bool ProtocolComponent::Awake()
    {
		rapidjson::Document jsonMapper;
		const std::string & dir = App::Get().GetConfigPath();	
		if (!FileHelper::ReadJsonFile(dir + "rpc.json", jsonMapper))
		{
			SayNoDebugFatal("not find file : " << dir << "");
			return false;///
		}

		auto iter1 = jsonMapper.MemberBegin();
		for (; iter1 != jsonMapper.MemberEnd(); iter1++)
		{
			const std::string service = iter1->name.GetString();
			rapidjson::Value& jsonValue = iter1->value;
			SayNoAssertRetFalse_F(jsonValue.IsObject());
			SayNoAssertRetFalse_F(jsonValue.HasMember("ID"));

			std::vector<ProtocolConfig *> methods;
			auto iter2 = jsonValue.MemberBegin();
			for (; iter2 != jsonValue.MemberEnd(); iter2++)
            {
                if (!iter2->value.IsObject())
                {
                    continue;
                }
                ProtocolConfig *protocolConfig = new ProtocolConfig();

				protocolConfig->ServiceName = service;
                protocolConfig->Method = iter2->name.GetString();
                SayNoAssertRetFalse_F(iter2->value.HasMember("ID"));

				protocolConfig->IsAsync = iter2->value["Async"].GetBool();
                protocolConfig->MethodId = (unsigned short) iter2->value["ID"].GetUint();


                if(iter2->value.HasMember("Request"))
                {
                    const rapidjson::Value & jsonValue = iter2->value["Request"];
                    if(!jsonValue.IsObject())
                    {
                        return false;
                    }
                    if(jsonValue.HasMember("Message"))
                    {
                        protocolConfig->RequestMessage = jsonValue["Message"].GetString();
                        Message *message = MessagePool::New(protocolConfig->RequestMessage);
                        if (message == nullptr)
                        {
                            SayNoDebugFatal("create " << protocolConfig->ResponseMessage << " failure");
                            return false;
                        }
                    }
                    if(jsonValue.HasMember("Component"))
                    {
                        protocolConfig->RequestHandler = jsonValue["Component"].GetString();
                    }
                }

                if(iter2->value.HasMember("Response"))
                {
                    const rapidjson::Value & jsonValue = iter2->value["Response"];
                    if(!jsonValue.IsObject())
                    {
                        return false;
                    }
                    if(jsonValue.HasMember("Message"))
                    {
                        protocolConfig->ResponseMessage = jsonValue["Message"].GetString();
                        Message *message = MessagePool::New(protocolConfig->ResponseMessage);
                        if (message == nullptr)
                        {
                            SayNoDebugFatal("create " << protocolConfig->ResponseMessage << " failure");
                            return false;
                        }
                    }
                    if(jsonValue.HasMember("Component"))
                    {
                        protocolConfig->ResponseHandler = jsonValue["Component"].GetString();
                    }
                }

                methods.push_back(protocolConfig);
                std::string name = service + "." + protocolConfig->Method;
                this->mProtocolNameMap.insert(std::make_pair(name, protocolConfig));
                this->mProtocolMap.insert(std::make_pair(protocolConfig->MethodId, protocolConfig));
            }
			this->mServiceMap.emplace(service, methods);
		}
        return true;
    }

	void ProtocolComponent::Start()
	{
			
	}


	bool ProtocolComponent::HasService(const std::string & service)
	{
		auto iter = this->mServiceMap.find(service);
		return iter != this->mServiceMap.end();
	}

	void ProtocolComponent::GetServices(std::vector<std::string> & services)
	{
		auto iter = this->mServiceMap.begin();
		for (; iter != this->mServiceMap.end(); iter++)
		{
			services.push_back(iter->first);
		}
	}

	bool ProtocolComponent::GetMethods(const std::string & service, std::vector<std::string> & methods)
	{
		auto iter = this->mServiceMap.find(service);
		if (iter == this->mServiceMap.end())
		{
			return false;
		}
		for (ProtocolConfig * config : iter->second)
		{
			methods.push_back(config->Method);
		}
		return true;
	}

	const ProtocolConfig *ProtocolComponent::GetProtocolConfig(unsigned short id) const
    {
        auto iter = this->mProtocolMap.find(id);
        return iter != this->mProtocolMap.end() ? iter->second : nullptr;
    }

    const ProtocolConfig *
    ProtocolComponent::GetProtocolConfig(const std::string &service, const std::string &method) const
    {
        std::string name = service + "." + method;
        auto iter = this->mProtocolNameMap.find(name);
        return iter != this->mProtocolNameMap.end() ? iter->second : nullptr;
    }
}
