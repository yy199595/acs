//
// Created by mac on 2022/4/12.
//

#include"MethodRegister.h"
#include"Entity/Actor/App.h"
#include"Rpc/Service/RpcService.h"
#include"Proto/Component/ProtoComponent.h"
namespace acs
{
	bool ServiceMethodRegister::AddMethod(std::unique_ptr<ServiceMethod> method)
	{
        ProtoComponent * protoComponent = App::GetProto();
        RpcService * serviceComponent = this->mComponent->Cast<RpcService>();
        LOG_CHECK_RET_FALSE(serviceComponent != nullptr);

        const std::string & name = method->GetName();
        const std::string & service = serviceComponent->GetName();
        std::string fullName = fmt::format("{0}.{1}", service, name);
        const RpcMethodConfig * config = RpcConfig::Inst()->GetMethodConfig(fullName);
        {
            if(config == nullptr)
            {
                LOG_ERROR("not find rpc config : {}", fullName);
                return false;
            }
			if(config->proto == rpc::Porto::Protobuf)
			{
				const std::string& request = config->request;
				const std::string& response = config->response;
				if (!request.empty() && protoComponent->Temp(request) == nullptr)
				{
					LOG_ERROR("rpc config {} request message:{}", config->fullname, request);
					return false;
				}
				if (!response.empty() && protoComponent->Temp(response) == nullptr)
				{
					LOG_ERROR("rpc config {} response message:{}", config->fullname, response);
					return false;
				}
			}
        }
		
		auto iter = this->mMethodMap.find(name);
		if (iter != this->mMethodMap.end())
		{
			LOG_FATAL("{} already exist", fullName);
			return false;
		}
		this->mMethodMap.emplace(name, std::move(method));
		return true;
	}

	ServiceMethod * ServiceMethodRegister::GetMethod(const string& name)
	{
		auto iter1 = this->mMethodMap.find(name);
		if(iter1 != this->mMethodMap.end())
		{
			return iter1->second.get();
		}
		return nullptr;
	}

	ServiceMethodRegister::ServiceMethodRegister(Component * component)
			: mComponent(component)
	{

	}
}

namespace acs
{
	HttpServiceMethod * HttpServiceRegister::GetMethod(const string& name)
	{    
		auto iter1 = this->mHttpMethodMap.find(name);
		return iter1 != this->mHttpMethodMap.end() ? iter1->second.get() : nullptr;
	}

    bool HttpServiceRegister::AddMethod(std::unique_ptr<HttpServiceMethod> method)
    {
		if (method == nullptr)
		{
			return false;
		}
        const std::string & name = method->GetName();
		auto iter = this->mHttpMethodMap.find(name);
		if (iter != this->mHttpMethodMap.end())
		{
			return false;
		}
		this->mHttpMethodMap.emplace(name, std::move(method));
        return true;
    }
}