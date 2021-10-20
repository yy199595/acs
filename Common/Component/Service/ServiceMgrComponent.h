#pragma once

#include<Protocol/s2s.pb.h>
#include<Component/Component.h>
#include<Other/MultiThreadQueue.h>
#include<Pool/ObjectPool.h>
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
        bool OnRequestMessage(const com::DataPacket_Request & message) final;

        virtual int GetPriority() { return 500; }
	private:
        void Invoke(ServiceMethod * method, com::DataPacket_Request * request);
    private:
        int mNodeId;
        class CoroutineComponent *mCorComponent;
        class ProtocolComponent * mProtocolComponent;
		class TcpNetSessionComponent *mNetSessionComponent;
		ObjectPool<com::DataPacket_Request> mRequestDataPool;
    };
}