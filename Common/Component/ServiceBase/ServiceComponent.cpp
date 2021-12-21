#include"ServiceComponent.h"
#include<Core/App.h>
#include <Method/LuaServiceMethod.h>
#include<Scene/RpcConfigComponent.h>
#ifdef __DEBUG__
#include"Other/ElapsedTimer.h"
#include"Pool/MessagePool.h"
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

    com::Rpc_Response *ServiceComponent::Invoke(const string &method, const com::Rpc_Request * request)
    {
        LocalObject<com::Rpc_Request> local(request);
        auto iter = this->mMethodMap.find(method);
        if (iter == this->mMethodMap.end())
        {
            return nullptr;
        }
#ifdef __DEBUG__
        ElapsedTimer elapsedTimer;
#endif
        ServiceMethod *serviceMethod = iter->second;
        com::Rpc_Response *response = new com::Rpc_Response();
        XCode code = serviceMethod->Invoke(*request, *response);
        if (request->rpcid() == 0)
        {
            delete response;
            return nullptr;
        }
        response->set_code((int) code);
        response->set_rpcid(request->rpcid());
        response->set_userid(request->userid());
#ifdef __DEBUG__
        std::string json;
        LOG_DEBUG("===============[rpc request]===============");
        LOG_DEBUG("[func] = " << this->GetServiceName() << "." << method);
        LOG_DEBUG("[time] = " << elapsedTimer.GetMs() << "ms");
        if (request->has_data() && Helper::Proto::GetJson(request->data(), json))
        {
            LOG_DEBUG("[request] = " << json);
        }
        if(response->has_data())
        {
            json.clear();
            if (Helper::Proto::GetJson(response->data(), json))
            {
                LOG_DEBUG("[response] = " << json);
            }
        }
#endif
        return response;
    }
}
