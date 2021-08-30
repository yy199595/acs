#pragma once

#include <memory>
#include <Component/Component.h>
#include <NetWork/PacketMapper.h>

using namespace std;
using namespace com;


namespace Sentry
{
	class LocalActionProxy;
    class NetWorkWaitCorAction;

    class ServiceBase : public Component
    {
    public:
        ServiceBase() {}

        virtual ~ServiceBase() {}

    public:

        virtual void Start() {};
		virtual bool Awake() { return true; }
        virtual bool IsLuaService() { return false; };
    public:      
		virtual const std::string &GetServiceName() = 0;
        virtual bool HasMethod(const std::string &method) = 0;	
		virtual void GetMethods(std::vector<LocalActionProxy*> & methods) = 0;

        virtual void OnRefreshService() {}; //刷新服务表调用
       
    public:
		virtual XCode InvokeMethod(PacketMapper *) = 0;      
    private:
        std::string mServiceName;
    };
}