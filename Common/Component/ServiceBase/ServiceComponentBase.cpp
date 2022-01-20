#include"ServiceComponentBase.h"
#include<Core/App.h>
#include <Method/LuaServiceMethod.h>
#include<Scene/RpcConfigComponent.h>
#ifdef __DEBUG__
#include"Other/ElapsedTimer.h"
#include"Pool/MessagePool.h"
#endif
namespace Sentry
{
	bool ServiceComponentBase::AddMethod(ServiceMethod * method)
    {
        auto *rpcConfigComponent = this->GetComponent<RpcConfigComponent>();
        if (rpcConfigComponent == nullptr)
        {
            return false;
        }
        const std::string &name = method->GetName();
        const std::string &service = this->GetServiceName();
        if (!rpcConfigComponent->HasServiceMethod(service, name))
        {
            LOG_FATAL(this->GetServiceName(), '.', name, " add failure");
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
            LOG_DEBUG("add new lua service ", service,'.', name);
            return true;
        }

        auto iter = this->mMethodMap.find(name);
        if (iter != this->mMethodMap.end())
        {
            LOG_FATAL(this->GetServiceName(), '.', name, " add failure");
            return false;
        }
        this->mMethodMap.emplace(name, method);
        LOG_DEBUG("add new c++ service method", service,'.', name);
        return true;
    }

    void ServiceComponentBase::GetSubMethods(std::vector<std::string> &methods)
    {
        auto iter = this->mSubMethodMap.begin();
        for(; iter != this->mSubMethodMap.end(); iter++)
        {
            methods.emplace_back(iter->first);
        }
    }

    void ServiceComponentBase::Publish(const std::string & func, const std::string &message)
    {
        auto iter = this->mSubMethodMap.find(func);
        if(iter != this->mSubMethodMap.end())
        {
            iter->second->OnPublish(message);
        }
    }

    ServiceMethod *ServiceComponentBase::GetMethod(const std::string &name)
    {
        auto iter = this->mLuaMethodMap.find(name);
        if(iter != this->mLuaMethodMap.end())
        {
            return iter->second;
        }
        auto iter1 = this->mMethodMap.find(name);
        return iter1 != this->mMethodMap.end() ? iter1->second : nullptr;
    }

    std::shared_ptr<com::Rpc_Response> ServiceComponentBase::Invoke(const string &method, std::shared_ptr<com::Rpc_Request> request)
    {
        ServiceMethod *serviceMethod = this->GetMethod(method);
        if (serviceMethod == nullptr)
        {
            return nullptr;
        }
#ifdef __DEBUG__
        ElapsedTimer elapsedTimer;
#endif
        std::shared_ptr<com::Rpc_Response> response(new com::Rpc_Response());
        XCode code = serviceMethod->Invoke(*request, *response);
        if (request->rpc_id() == 0)
        {
            return nullptr;
        }
        response->set_code((int) code);
        response->set_rpc_id(request->rpc_id());
        response->set_user_id(request->user_id());
#ifdef __DEBUG__
        LOG_DEBUG("===============[rpc request]===============");
        LOG_DEBUG("[func] =", this->GetServiceName(), '.', method);
        LOG_DEBUG("[time] = [", elapsedTimer.GetMs(), "ms]");
        if (request->has_data())
        {
            LOG_DEBUG("[request] = ", Helper::Proto::ToJson(request->data()));
        }
        if (response->has_data())
        {
            LOG_DEBUG("[response] = ", Helper::Proto::ToJson(response->data()));
        }
#endif
        return response;
    }
}
