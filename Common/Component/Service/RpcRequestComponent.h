#pragma once

#include<Protocol/s2s.pb.h>
#include<Component/Component.h>
#include<Other/MultiThreadQueue.h>
#include<Pool/ObjectPool.h>
namespace GameKeeper
{
    class ServiceComponent;

    class LocalServiceComponent;
    
    class LuaServiceComponent;

	class ServiceMethod;

    class RpcRequestComponent : public Component, public IProtoRequest
    {
    public:
		RpcRequestComponent() = default;
        ~RpcRequestComponent() = default;

    protected:
        bool Awake() final;

    public:
        bool OnRequest(const com::DataPacket_Request & message) final;

        virtual int GetPriority() const { return 500; }
	private:
        void Invoke(ServiceMethod * method, com::DataPacket_Request * request);
    private:
        int mNodeId;
        std::string mMessageBuffer;
        class RpcComponent *mRpcComponent;
        com::DataPacket_Response mResponse;
        class CoroutineComponent *mCorComponent;
        class ProtocolComponent * mProtocolComponent;
		ObjectPool<com::DataPacket_Request> mRequestDataPool;
    };
}