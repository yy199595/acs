#include "RpcConfigComponent.h"

#include <Core/App.h>
#include <Util/FileHelper.h>
#include <Util/StringHelper.h>
#include <rapidjson/document.h>
#include <Pool/MessagePool.h>
#include <google/protobuf/util/json_util.h>
namespace GameKeeper
{
	bool RpcConfigComponent::Awake()
    {
        return this->OnLoadConfig();
    }

    bool RpcConfigComponent::OnLoadConfig()
    {
        rapidjson::Document jsonMapper;
        const std::string path = App::Get().GetConfigPath() + "rpc.json";
        if (!FileHelper::ReadJsonFile(path, jsonMapper))
        {
            GKDebugFatal("not find file : " << path << "");
            return false;///
        }

        this->mServiceMap.clear();
        this->mProtocolNameMap.clear();

        auto iter1 = jsonMapper.MemberBegin();
        for (; iter1 != jsonMapper.MemberEnd(); iter1++)
        {
            const std::string service = iter1->name.GetString();
            rapidjson::Value &jsonValue = iter1->value;
            GKAssertRetFalse_F(jsonValue.IsObject());

            std::vector<ProtocolConfig> methods;
            auto iter2 = jsonValue.MemberBegin();
            for (; iter2 != jsonValue.MemberEnd(); iter2++)
            {
                if (!iter2->value.IsObject())
                {
                    continue;
                }
                ProtocolConfig protocolConfig;
                protocolConfig.Timeout = 0;
                protocolConfig.Service = service;
                protocolConfig.Method = iter2->name.GetString();
                protocolConfig.MethodId = iter2->value["Id"].GetInt();
                protocolConfig.IsAsync = iter2->value["Async"].GetBool();
                if(iter2->value.HasMember("Timeout"))
                {
                    protocolConfig.Timeout = iter2->value["Timeout"].GetInt();
                }
                if (iter2->value.HasMember("Request"))
                {
                    const rapidjson::Value &jsonValue = iter2->value["Request"];
                    GKAssertRetFalse_F(jsonValue.IsString());
                    protocolConfig.Request = jsonValue.GetString();
                    if(MessagePool::New(protocolConfig.Request) == nullptr)
                    {
                        GKDebugFatal("create " << protocolConfig.Response << " failure");
                        return false;
                    }
                }

                if (iter2->value.HasMember("Response"))
                {
                    const rapidjson::Value &jsonValue = iter2->value["Response"];
                    GKAssertRetFalse_F(jsonValue.IsString());
                    protocolConfig.Response = jsonValue.GetString();               
                    if (MessagePool::New(protocolConfig.Response) == nullptr)
                    {
                        GKDebugFatal("create " << protocolConfig.Response << " failure");
                        return false;
                    }
                }

                methods.push_back(protocolConfig);
                std::string name = service + "." + protocolConfig.Method;

                this->mProtocolNameMap.emplace(name, protocolConfig);
                this->mProtocolIdMap.emplace(protocolConfig.MethodId, protocolConfig);
            }
            this->mServiceMap.emplace(service, methods);
        }
        return this->LoadCodeConfig();
    }

    bool RpcConfigComponent::LoadCodeConfig()
    {
        std::vector<std::string> lines;
        const std::string path = App::Get().GetWorkPath();
        if (!FileHelper::ReadTxtFile(path + "XCode/XCode.csv", lines))
        {
            GKDebugError("not find file " << path + "XCode/XCode.csv");
            return false;
        }
        std::vector<std::string> res;
        for (int index = 1; index < lines.size(); index++)
        {
            res.clear();
            const std::string &line = lines[index];
            StringHelper::SplitString(line, "\t", res);
            if (res.size() == 2)
            {
                CodeConfig codeConfig;
                codeConfig.Name = res[0];
                codeConfig.Desc = res[1];
                codeConfig.Code = index - 1;
                this->mCodeDescMap.emplace(codeConfig.Code, codeConfig);
            }
        }
        return true;
    }

    const ProtocolConfig *RpcConfigComponent::GetProtocolConfig(int methodId) const
    {
        auto iter = this->mProtocolIdMap.find(methodId);
        return iter != this->mProtocolIdMap.end() ? &iter->second : nullptr;
    }

    const CodeConfig *RpcConfigComponent::GetCodeConfig(int code) const
    {
        auto iter = this->mCodeDescMap.find(code);
        return iter != this->mCodeDescMap.end() ? &iter->second : nullptr;
    }

	bool RpcConfigComponent::HasService(const std::string & service)
	{
		auto iter = this->mServiceMap.find(service);
		return iter != this->mServiceMap.end();
	}

	void RpcConfigComponent::GetServices(std::vector<std::string> & services)
	{
		auto iter = this->mServiceMap.begin();
		for (; iter != this->mServiceMap.end(); iter++)
		{
			services.push_back(iter->first);
		}
	}

    void RpcConfigComponent::DebugCode(XCode code)
    {
        auto iter = this->mCodeDescMap.find(code);
        if(iter != this->mCodeDescMap.end())
        {
            GKDebugError("code = [" << iter->second.Name << ":" << iter->second.Desc <<"]");
        }
    }

	bool RpcConfigComponent::HasServiceMethod(const std::string & service, const std::string & method)
	{
		auto iter = this->mServiceMap.find(service);
		if (iter == this->mServiceMap.end())
		{
			return false;
		}
		for (const auto& config : iter->second)
		{
			if (config.Method == method)
			{
				return true;
			}
		}
		return false;
	}

	bool RpcConfigComponent::GetMethods(const std::string & service, std::vector<std::string> & methods)
	{
		auto iter = this->mServiceMap.find(service);
		if (iter == this->mServiceMap.end())
		{
			return false;
		}
		for (const ProtocolConfig & config : iter->second)
		{
			methods.push_back(config.Method);
		}
		return true;
	}

    const ProtocolConfig * RpcConfigComponent::GetProtocolConfig(const std::string &fullName) const
    {
        auto iter = this->mProtocolNameMap.find(fullName);
        return iter != this->mProtocolNameMap.end() ? &iter->second : nullptr;
    }
}
