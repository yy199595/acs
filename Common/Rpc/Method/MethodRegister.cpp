//
// Created by mac on 2022/4/12.
//

#include"MethodRegister.h"
#include"Entity/Actor/App.h"
#include"Rpc/Service/RpcService.h"
#include"Proto/Component/ProtoComponent.h"
namespace Tendo
{
	bool ServiceMethodRegister::AddMethod(std::shared_ptr<ServiceMethod> method)
	{
        ProtoComponent * protoComponent = App::Inst()->GetProto();
        RpcService * serviceComponent = this->mComponent->Cast<RpcService>();
        LOG_CHECK_RET_FALSE(serviceComponent != nullptr);

        const std::string & name = method->GetName();
        const std::string & service = serviceComponent->GetName();
        std::string fullName = fmt::format("{0}.{1}", service, name);
        const RpcMethodConfig * config = RpcConfig::Inst()->GetMethodConfig(fullName);
        {
            if(config == nullptr)
            {
                LOG_ERROR("not find rpc config : [" << fullName << "]");
                return false;
            }
            const std::string & request = config->Request;
            const std::string & response = config->Response;
            if(!request.empty() && !protoComponent->HasMessage(request))
            {
                LOG_ERROR("rpc config error [" << config->FullName << "] request message");
                return false;
            }
            if(!response.empty() && !protoComponent->HasMessage(response))
            {
                LOG_ERROR("rpc config error [" << config->FullName << "] response message");
                return false;
            }
        }
		
		auto iter = this->mMethodMap.find(name);
		if (iter != this->mMethodMap.end())
		{
			LOG_FATAL(this->mComponent->GetName() << "." << name << " already exist");
			return false;
		}
		this->mMethodMap.emplace(name, method);
		//LOG_DEBUG("add new c++ service method [" << this->mService <<'.' << name << ']');
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

namespace Tendo
{
	HttpServiceMethod * HttpServiceRegister::GetMethod(const string& name)
	{    
		auto iter1 = this->mHttpMethodMap.find(name);
		return iter1 != this->mHttpMethodMap.end() ? iter1->second.get() : nullptr;
	}

    bool HttpServiceRegister::AddMethod(std::shared_ptr<HttpServiceMethod> method)
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
		this->mHttpMethodMap.emplace(name, method);
        return true;
    }
}