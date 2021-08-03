#pragma once

#include <memory>
#include <Manager/Manager.h>
#include <Protocol/com.pb.h>
#include <NetWork/NetMessageProxy.h>

using namespace std;
using namespace com;


namespace Sentry
{
    class NetWorkWaitCorAction;

    class ServiceBase : public Object
    {
    public:
        ServiceBase();

        virtual ~ServiceBase() {}

    public:
        virtual bool OnInit() { return true; }

        virtual void OnInitComplete() {};

        virtual bool IsLuaService() { return false; };
    public:
        bool IsInit() { return this->mIsInit; }

        virtual bool HasMethod(const std::string &method) = 0;

        virtual void OnRefreshService() {}; //刷新服务表调用
        const std::string &GetServiceName() { return this->mServiceName; };
    public:
        virtual bool InvokeMethod(NetMessageProxy *) = 0;

        virtual bool InvokeMethod(const std::string &address, NetMessageProxy *) = 0;

        virtual void GetServiceList(std::vector<shared_ptr<LocalActionProxy>> &service) = 0;

    public:
        void InitService(const std::string &name);

    private:
        bool mIsInit;
        std::string mServiceName;
    };
}