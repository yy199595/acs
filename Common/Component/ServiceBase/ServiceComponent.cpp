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

        if(method->IsLuaMethod())
        {
            auto iter = this->mLuaMethodMap.find(name);
            if(iter != this->mLuaMethodMap.end())
            {
                delete iter->second;
                this->mLuaMethodMap.erase(iter);
            }
            this->mLuaMethodMap.emplace(name, method);
            return true;
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

    ServiceMethod *ServiceComponent::GetMethod(const std::string &name)
    {
        auto iter = this->mLuaMethodMap.find(name);
        if(iter != this->mLuaMethodMap.end())
        {
            return iter->second;
        }
        auto iter1 = this->mMethodMap.find(name);
        return iter1 != this->mMethodMap.end() ? iter1->second : nullptr;
    }

    std::shared_ptr<com::Rpc_Response> ServiceComponent::Invoke(const string &method, std::shared_ptr<com::Rpc_Request> request)
    {
        ServiceMethod *serviceMethod = this->GetMethod(method);
        if(serviceMethod == nullptr)
        {
            return nullptr;
        }
#ifdef __DEBUG__
        ElapsedTimer elapsedTimer;
#endif
        std::shared_ptr<com::Rpc_Response> response(new com::Rpc_Response());
        XCode code = serviceMethod->Invoke(*request, *response);
        if (request->rpcid() == 0)
        {
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
