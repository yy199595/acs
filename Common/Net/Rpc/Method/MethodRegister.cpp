//
// Created by mac on 2022/4/12.
//

#include"MethodRegister.h"
#include"Entity/Actor/App.h"
#include"Rpc/Service/RpcService.h"
#include"Rpc/Config/ServiceConfig.h"
#include"Proto/Component/ProtoComponent.h"
namespace acs
{
	bool ServiceMethodRegister::AddMethod(std::unique_ptr<ServiceMethod> method)
	{
		if(this->Has(method->GetName()))
		{
			LOG_FATAL("{} already exist", method->GetName());
			return false;
		}
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
			if(config->proto == rpc::proto::pb)
			{
				const std::string& request = config->request;
				const std::string& response = config->response;
				if (!request.empty() && !protoComponent->Has(request))
				{
					LOG_ERROR("rpc config {} request message:{}", config->fullname, request);
					return false;
				}
				if (!response.empty() && !protoComponent->Has(response))
				{
					LOG_ERROR("rpc config {} response message:{}", config->fullname, response);
					return false;
				}
			}
        }
		this->mMethodMap.emplace_back(std::move(method));
		return true;
	}

	ServiceMethod * ServiceMethodRegister::GetMethod(const std::string& name)
	{
		auto iter1 = std::find_if(this->mMethodMap.begin(), this->mMethodMap.end(),
				[&name](std::unique_ptr<ServiceMethod> & method) {
			return method->GetName() == name;
		});
		return iter1 == this->mMethodMap.end() ? nullptr : (*iter1).get();
	}

	ServiceMethodRegister::ServiceMethodRegister(Component * component)
			: mComponent(component)
	{

	}
}

namespace acs
{
	HttpServiceMethod * HttpServiceRegister::GetMethod(const std::string& name)
	{
		auto iter = std::find_if(this->mHttpMethods.begin(), this->mHttpMethods.end(),
				[&name](const std::unique_ptr<HttpServiceMethod> & method) {
					return method->GetName() == name;
		});
		return iter != this->mHttpMethods.end() ? (*iter).get() : nullptr;
	}

    bool HttpServiceRegister::AddMethod(std::unique_ptr<HttpServiceMethod> method)
    {
		if (method == nullptr || this->Has(method->GetName()))
		{
			return false;
		}
		this->mHttpMethods.emplace_back(std::move(method));
        return true;
    }
}