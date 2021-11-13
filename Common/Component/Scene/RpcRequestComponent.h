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
        ~RpcRequestComponent() final = default;

    protected:
        bool Awake() final;

    public:
        bool OnRequest(const com::Rpc_Request & message) final;

        virtual int GetPriority() const { return 500; }
	private:
        void Invoke(ServiceMethod * method, const com::Rpc_Request * request);
    private:
        int mNodeId;
        std::string mMessageBuffer;
        class RpcComponent *mRpcComponent;
        class CoroutineComponent *mCorComponent;
        class RpcProtoComponent * mProtocolComponent;
    };
}