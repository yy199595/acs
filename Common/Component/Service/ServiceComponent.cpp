#include"Service/ServiceComponent.h"
#include<Core/App.h>
#include <Method/LuaServiceMethod.h>
#include"Method/JsonServiceMethod.h"
#include<Scene/RpcConfigComponent.h>
#ifdef __DEBUG__
#include"Other/ElapsedTimer.h"
#include<google/protobuf/util/json_util.h>
#endif
namespace GameKeeper
{
	bool ServiceComponent::AddMethod(ServiceMethod * method)
    {
        auto *rpcConfigComponent = App::Get().GetComponent<RpcConfigComponent>();
        if (rpcConfigComponent == nullptr)
        {
            return false;
        }
        const std::string &name = method->GetName();
        const std::string &service = this->GetServiceName();
        if (!rpcConfigComponent->HasServiceMethod(service, name))
        {
            LOG_FATAL(this->GetServiceName() << "." << name << " not config");
            return false;
        }

        auto iter = this->mMethodMap.find(name);
        if (iter != this->mMethodMap.end())
        {
            LOG_FATAL(this->GetServiceName() << "." << name << " add failure");
            return false;
        }
        this->mMethodMap.emplace(name, method);
        return true;
    }
	bool ServiceComponent::HasProtoMethod(const std::string & method)
	{
		auto iter1 = this->mMethodMap.find(method);
		return iter1 != this->mMethodMap.end();
	}
	ServiceMethod * ServiceComponent::GetProtoMethod(const std::string & method)
	{
		auto iter1 = this->mMethodMap.find(method);
		return iter1 != this->mMethodMap.end() ? iter1->second : nullptr;
	}

    com::Rpc_Response *ServiceComponent::Invoke(const string &method, const com::Rpc_Request * request)
    {
        LocalObject<com::Rpc_Request> local(request);
        auto iter = this->mMethodMap.find(method);
        if (iter == this->mMethodMap.end()) {
            return nullptr;
        }
#ifdef __DEBUG__
        ElapsedTimer elapsedTimer;
#endif
        ServiceMethod *serviceMethod = iter->second;
        com::Rpc_Response *response = new com::Rpc_Response();
        XCode code = serviceMethod->Invoke(*request, *response);
        if (request->rpcid() == 0) {
            delete response;
            return nullptr;
        }
        response->set_code((int) code);
        response->set_rpcid(request->rpcid());
        response->set_userid(request->userid());
#ifdef __DEBUG__
        LOG_DEBUG("===============[rpc request]===============");
        LOG_DEBUG("func = " << this->GetServiceName() << "." << method);
        if(request->has_data())
        {
            std::string json;
            util::MessageToJsonString(request->data(), &json);
            LOG_DEBUG("request = " << json);
        }
        if(response->has_data())
        {
            std::string json;
            util::MessageToJsonString(response->data(), &json);
            LOG_DEBUG("response = " << json);
        }
        LOG_DEBUG("time = [" << elapsedTimer.GetMs() << "ms]");
#endif
        return response;
    }
}
