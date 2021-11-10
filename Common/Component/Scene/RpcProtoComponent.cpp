#include "RpcProtoComponent.h"

#include <Core/App.h>
#include <Util/FileHelper.h>
#include <Util/StringHelper.h>
#include <rapidjson/document.h>
#include <google/protobuf/util/json_util.h>
namespace GameKeeper
{
	bool RpcProtoComponent::Awake()
    {
        const std::string path1 = App::Get().GetConfigPath() + "rpc.json";
        const std::string path2 = App::Get().GetConfigPath() + "http.json";
        GKAssertRetFalse_F(this->LoadTcpServiceConfig(path1));
        return true;
    }

    bool RpcProtoComponent::LoadTcpServiceConfig(const std::string &path)
    {
        rapidjson::Document jsonMapper;
        if (!FileHelper::ReadJsonFile(path, jsonMapper))
        {
            GKDebugFatal("not find file : " << path << "");
            return false;///
        }

        auto iter1 = jsonMapper.MemberBegin();
        for (; iter1 != jsonMapper.MemberEnd(); iter1++)
        {
            const std::string service = iter1->name.GetString();
            rapidjson::Value& jsonValue = iter1->value;
            GKAssertRetFalse_F(jsonValue.IsObject());
            GKAssertRetFalse_F(jsonValue.HasMember("ID"));

            std::vector<ProtocolConfig *> methods;
            auto iter2 = jsonValue.MemberBegin();
            for (; iter2 != jsonValue.MemberEnd(); iter2++)
            {
                if (!iter2->value.IsObject())
                {
                    continue;
                }
                auto protocolConfig = new ProtocolConfig();

                protocolConfig->ServiceName = service;
                protocolConfig->Method = iter2->name.GetString();
                GKAssertRetFalse_F(iter2->value.HasMember("ID"));

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
                        if(!this->AddProto(protocolConfig->RequestMessage))
                        {
                            GKDebugFatal("create " << protocolConfig->ResponseMessage << " failure");
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
                        if(!this->AddProto(protocolConfig->ResponseMessage))
                        {
                            GKDebugFatal("create " << protocolConfig->ResponseMessage << " failure");
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

    Message *RpcProtoComponent::NewProtoMessage(const std::string &name)
    {
        if(name.empty())
        {
            return nullptr;
        }
        if(this->mLock.try_lock())
        {
            std::lock_guard<std::mutex> lock(this->mLock);
            auto iter = this->mProtoMap.find(name);
            if (iter != this->mProtoMap.end())
            {
                Message *message = iter->second;
                return message->New();
            }
        }
        return nullptr;
    }

	bool RpcProtoComponent::HasService(const std::string & service)
	{
		auto iter = this->mServiceMap.find(service);
		return iter != this->mServiceMap.end();
	}

	void RpcProtoComponent::GetServices(std::vector<std::string> & services)
	{
		auto iter = this->mServiceMap.begin();
		for (; iter != this->mServiceMap.end(); iter++)
		{
			services.push_back(iter->first);
		}
	}

	bool RpcProtoComponent::GetMethods(const std::string & service, std::vector<std::string> & methods)
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

	const ProtocolConfig *RpcProtoComponent::GetProtocolConfig(unsigned short id) const
    {
        auto iter = this->mProtocolMap.find(id);
        return iter != this->mProtocolMap.end() ? iter->second : nullptr;
    }

    bool RpcProtoComponent::AddProto(const std::string &name)
    {
        const DescriptorPool *pDescriptorPool = DescriptorPool::generated_pool();
        const Descriptor *pDescriptor = pDescriptorPool->FindMessageTypeByName(name);
        if (pDescriptor == nullptr)
        {
            return false;
        }
        MessageFactory *factory = MessageFactory::generated_factory();
        const Message *pMessage = factory->GetPrototype(pDescriptor);
        this->mProtoMap.emplace(name, pMessage->New());
        return true;
    }


    const ProtocolConfig *
    RpcProtoComponent::GetProtocolConfig(const std::string &service, const std::string &method) const
    {
        std::string name = service + "." + method;
        auto iter = this->mProtocolNameMap.find(name);
        return iter != this->mProtocolNameMap.end() ? iter->second : nullptr;
    }
}
