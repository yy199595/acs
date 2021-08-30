#pragma once

#include<Protocol/s2s.pb.h>
#include<Scene/SceneNetProxyComponent.h>
#include<Other/DoubleBufferQueue.h>


namespace Sentry
{
    class ServiceBase;

    class LocalService;
    
    class LocalLuaService;

    class ServiceMgrComponent : public Component
    {
    public:
        ServiceMgrComponent() {}

        ~ServiceMgrComponent() {}

    protected:
        bool Awake() final;

    public:
        bool HandlerMessage(PacketMapper *messageData);

        bool HandlerMessage(const std::string &adress, PacketMapper *messageData);
		
		virtual int GetPriority() { return 1; }
	private:	
		void Invoke1(PacketMapper *messageData);
		void Invoke2(const std::string &adress, PacketMapper *messageData);
    private:
        int mNodeId;

        class CoroutineComponent *mCorComponent;

        class SceneNetProxyComponent *mNetProxyManager;
    };
}