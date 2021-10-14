#pragma once

#include<Protocol/s2s.pb.h>
#include<Scene/NetProxyComponent.h>
#include<Other/DoubleBufferQueue.h>


namespace Sentry
{
    class ServiceComponent;

    class LocalServiceComponent;
    
    class LuaServiceComponent;

	class ServiceMethod;

    class ServiceMgrComponent : public Component, public IRequestMessageHandler
    {
    public:
        ServiceMgrComponent() {}

        ~ServiceMgrComponent() {}

    protected:
        bool Awake() final;

    public:
        bool OnRequestMessage(const std::string & address, SharedMessage message) final;

        virtual int GetPriority() { return 500; }
	private:
        void Invoke(ServiceMethod * method, std::string &address, SharedMessage message);
    private:
        int mNodeId;
        class CoroutineComponent *mCorComponent;
        class NetProxyComponent *mNetProxyManager;
        class ProtocolComponent * mProtocolComponent;
    };
}