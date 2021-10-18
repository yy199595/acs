#pragma once

#include<Protocol/s2s.pb.h>
#include<Component/Component.h>
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
        class ProtocolComponent * mProtocolComponent;
		class TcpNetSessionComponent *mNetSessionComponent;
    };
}