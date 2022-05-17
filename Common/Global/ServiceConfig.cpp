#include "ServiceConfig.h"

#include "App/App.h"
#include "Util/FileHelper.h"
#include "Util/StringHelper.h"
#include "rapidjson/document.h"
#include "Pool/MessagePool.h"
#include "google/protobuf/util/json_util.h"
namespace Sentry
{
	bool ServiceConfig::LoadConfig(const std::string & path)
	{
		rapidjson::Document jsonMapper;
		if (!Helper::File::ReadJsonFile(path, jsonMapper))
		{
			LOG_FATAL("not find file " << path);
			return false;///
		}

		this->mRpcNameMap.clear();
		this->mHttpPathMap.clear();
		auto iter1 = jsonMapper.MemberBegin();
		for (; iter1 != jsonMapper.MemberEnd(); iter1++)
		{
			rapidjson::Value& jsonValue = iter1->value;
			LOG_CHECK_RET_FALSE(jsonValue.IsObject());
			const std::string service = iter1->name.GetString();

			std::vector<std::string> methods;
			auto iter2 = jsonValue.MemberBegin();
			for (; iter2 != jsonValue.MemberEnd(); iter2++)
			{
				if (!iter2->value.IsObject())
				{
					continue;
				}
				const std::string method = iter2->name.GetString();
				if(!this->LoadRpcInterface(service, method, iter2->value))
				{
					if(!this->LoadHttpInterface(service, method, iter2->value))
					{
						return false;
					}
				}
				methods.emplace_back(method);
			}
			this->mServices.emplace_back(service);
			this->mMethods.emplace(service, methods);
		}
		return this->LoadCodeConfig();
	}


	bool ServiceConfig::LoadRpcInterface(const std::string & service,const std::string & method, const rapidjson::Value& jsonValue)
	{
		if(!jsonValue.HasMember("Id"))
		{
			return false;
		}
		RpcInterfaceConfig * rpcInterfaceConfig = new RpcInterfaceConfig();
		rpcInterfaceConfig->Timeout = 0;
		rpcInterfaceConfig->Method = method;
		rpcInterfaceConfig->Service = service;
		rpcInterfaceConfig->Type = jsonValue["Type"].GetString();
		rpcInterfaceConfig->InterfaceId = jsonValue["Id"].GetInt();
		rpcInterfaceConfig->IsAsync = jsonValue["Async"].GetBool();
		rpcInterfaceConfig->FullName = fmt::format("{0}.{1}", service, method);
		if(jsonValue.HasMember("CallWay"))
		{
			rpcInterfaceConfig->CallWay = jsonValue["CallWay"].GetString();
		}
		if (jsonValue.HasMember("Timeout"))
		{
			rpcInterfaceConfig->Timeout = jsonValue["Timeout"].GetInt();
		}
		if (jsonValue.HasMember("Request"))
		{
			rpcInterfaceConfig->Request = jsonValue["Request"].GetString();
			if (Helper::Proto::New(rpcInterfaceConfig->Request) == nullptr)
			{
				LOG_FATAL("create " << rpcInterfaceConfig->Request << " failure");
				return false;
			}
		}

		if (jsonValue.HasMember("Writer"))
		{
			rpcInterfaceConfig->Response = jsonValue["Writer"].GetString();
			if (Helper::Proto::New(rpcInterfaceConfig->Response) == nullptr)
			{
				LOG_FATAL("create [" << rpcInterfaceConfig->Response << "] failure");
				return false;
			}
		}
		this->mRpcNameMap.emplace(rpcInterfaceConfig->FullName, rpcInterfaceConfig);
		this->mRpcIdMap.emplace(rpcInterfaceConfig->InterfaceId, rpcInterfaceConfig);
		return true;
	}
	bool ServiceConfig::LoadCodeConfig()
	{
		std::string path;
		std::vector<std::string> lines;
		auto& config = App::Get()->GetConfig();
		LOG_CHECK_RET_FALSE(config.GetMember("path", "code", path));
		LOG_CHECK_RET_FALSE(Helper::File::ReadTxtFile(path, lines));
		std::vector<std::string> res;
		for (int index = 1; index < lines.size(); index++)
		{
			res.clear();
			const std::string& line = lines[index];
			Helper::String::SplitString(line, "\t", res);
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

	bool ServiceConfig::LoadHttpInterface(const std::string& service, const std::string& method,
			const rapidjson::Value& jsonValue)
	{
		if(!jsonValue.HasMember("Path"))
		{
			return false;
		}
		HttpInterfaceConfig * httpInterfaceConfig = new HttpInterfaceConfig();

		httpInterfaceConfig->Service = service;
		httpInterfaceConfig->Method = method;
		httpInterfaceConfig->Path = jsonValue["Path"].GetString();
		httpInterfaceConfig->Type = jsonValue["Type"].GetString();
		httpInterfaceConfig->Content = jsonValue["Content"].GetString();
		this->mHttpPathMap.emplace(httpInterfaceConfig->Path, httpInterfaceConfig);
		return true;
	}

	const RpcInterfaceConfig* ServiceConfig::GetInterfaceConfig(int methodId) const
	{
		auto iter = this->mRpcIdMap.find(methodId);
		return iter != this->mRpcIdMap.end() ? iter->second : nullptr;
	}

	const CodeConfig* ServiceConfig::GetCodeConfig(int code) const
	{
		auto iter = this->mCodeDescMap.find(code);
		return iter != this->mCodeDescMap.end() ? &iter->second : nullptr;
	}
#ifdef __DEBUG__
	void ServiceConfig::DebugCode(XCode code)
	{
		auto iter = this->mCodeDescMap.find((int)code);
		if (iter != this->mCodeDescMap.end())
		{
			LOG_ERROR("code = " << iter->second.Name << ':' << iter->second.Desc);
		}
	}

	std::string ServiceConfig::GetCodeDesc(XCode code) const
	{
		auto iter = this->mCodeDescMap.find((int)code);
		if (iter != this->mCodeDescMap.end())
		{
			return iter->second.Name + ":" + iter->second.Desc;
		}
		return std::string("");
	}

#endif

	bool ServiceConfig::HasServiceMethod(const std::string& service, const std::string& method) const
	{
		std::string fullName = fmt::format("{0}.{1}", service, method);
		return this->GetInterfaceConfig(fullName) != nullptr;
	}

	void ServiceConfig::GetMethods(const std::string& service, std::vector<std::string>& methods) const
	{
		auto iter = this->mMethods.find(service);
		if(iter != this->mMethods.end())
		{
			for(const std::string & name : iter->second)
			{
				methods.emplace_back(name);
			}
		}
	}

	const RpcInterfaceConfig* ServiceConfig::GetInterfaceConfig(const std::string& fullName) const
	{
		auto iter = this->mRpcNameMap.find(fullName);
		return iter != this->mRpcNameMap.end() ? iter->second : nullptr;
	}

	void ServiceConfig::GetService(std::vector<std::string>& services)
	{
		for(const std::string & name : this->mServices)
		{
			services.emplace_back(name);
		}
	}

	const HttpInterfaceConfig* ServiceConfig::GetHttpIterfaceConfig(const std::string& path) const
	{
		auto iter = this->mHttpPathMap.find(path);
		return iter != this->mHttpPathMap.end() ? iter->second : nullptr;
	}
}
