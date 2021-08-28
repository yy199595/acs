#pragma once

#include <memory>
#include <Component/Component.h>
#include <NetWork/NetMessageProxy.h>

using namespace std;
using namespace com;


namespace Sentry
{
    class NetWorkWaitCorAction;

    class ServiceBase : public Component
    {
    public:
        ServiceBase() {}

        virtual ~ServiceBase() {}

    public:
        virtual bool Awake() { return true; }

        virtual void Start() {};

        virtual bool IsLuaService() { return false; };
    public:
       
        virtual bool HasMethod(const std::string &method) = 0;

        virtual void OnRefreshService() {}; //刷新服务表调用
        const std::string &GetServiceName() { return this->mServiceName; };
    public:
		virtual XCode InvokeMethod(NetMessageProxy *) = 0;      
    private:
        std::string mServiceName;
    };
}