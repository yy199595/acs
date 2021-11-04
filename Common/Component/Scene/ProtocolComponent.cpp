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
        const std::string path1 = App::Get().GetConfigPath() + "rpc.json";
        const std::string path2 = App::Get().GetConfigPath() + "http.json";
        GKAssertRetFalse_F(this->LoadTcpServiceConfig(path1));
        GKAssertRetFalse_F(this->LoadHttpServiceConfig(path2));
        return true;
    }

    bool ProtocolComponent::LoadTcpServiceConfig(const std::string &path)
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
                        Message *message = MessagePool::New(protocolConfig->RequestMessage);
                        if (message == nullptr)
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
                        Message *message = MessagePool::New(protocolConfig->ResponseMessage);
                        if (message == nullptr)
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

    const HttpServiceConfig *ProtocolComponent::GetHttpConfig(const std::string &path) const
    {
        auto iter = this->mHttpConfigMap.find(path);
        return iter != this->mHttpConfigMap.end() ? iter->second : nullptr;
    }

    bool ProtocolComponent::LoadHttpServiceConfig(const std::string &path)
    {
        rapidjson::Document jsonMapper;
        if (!FileHelper::ReadJsonFile(path, jsonMapper))
        {
            GKDebugFatal("not find file : " << path << "");
            return false;
        }

        auto iter1 = jsonMapper.MemberBegin();
        for (; iter1 != jsonMapper.MemberEnd(); iter1++)
        {
            rapidjson::Value & jsonValue = iter1->value;
            if(!jsonValue.IsObject())
            {
                return false;
            }
            auto httpServiceConfig = new HttpServiceConfig();
            httpServiceConfig->Path = iter1->name.GetString();
            httpServiceConfig->Method = jsonValue["Method"].GetString();
            httpServiceConfig->Service = jsonValue["Service"].GetString();
            httpServiceConfig->ContentType = jsonValue["ContentType"].GetString();

            if(jsonValue.HasMember("Fields") && jsonValue["Fields"].IsArray())
            {
               for(unsigned int index = 0; index < jsonValue["Fields"].Size();index++)
               {
                  httpServiceConfig->HeardFields.insert(jsonValue["Fields"][index].GetString());
               }
            }
            this->mHttpConfigMap.emplace(httpServiceConfig->Path, httpServiceConfig);
        }
        return true;
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
