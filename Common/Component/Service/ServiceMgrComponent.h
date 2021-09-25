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

    class ServiceMgrComponent : public Component
    {
    public:
        ServiceMgrComponent() {}

        ~ServiceMgrComponent() {}

    protected:
        bool Awake() final;

    public:
        bool HandlerMessage(PacketMapper *messageData);

		virtual int GetPriority() { return 500; }
	private:	
		std::string GetJson(PacketMapper * messageData);
        void Invoke(ServiceMethod * method, PacketMapper *messageData);
    private:
        int mNodeId;

        class CoroutineComponent *mCorComponent;

        class NetProxyComponent *mNetProxyManager;
    };
}