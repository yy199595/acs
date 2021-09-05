#pragma once

#include<Protocol/s2s.pb.h>
#include<Scene/SceneNetProxyComponent.h>
#include<Other/DoubleBufferQueue.h>


namespace Sentry
{
    class ServiceBase;

    class LocalService;
    
    class LuaServiceProxy;

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

		virtual int GetPriority() { return 1; }
	private:	
		std::string GetJson(PacketMapper * messageData);
		void Invoke(ServiceMethod * method, PacketMapper *messageData);		
    private:
        int mNodeId;

        class CoroutineComponent *mCorComponent;

        class SceneNetProxyComponent *mNetProxyManager;
    };
}