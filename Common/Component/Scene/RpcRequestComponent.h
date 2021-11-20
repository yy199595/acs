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

    class RpcRequestComponent : public Component, public IProtoRequest, IJsonRequest
    {
    public:
		RpcRequestComponent() = default;
        ~RpcRequestComponent() final = default;

    protected:
        bool Awake() final;

    public:
        bool OnRequest(const com::Rpc_Request & message) final;
		bool OnRequest(const class RapidJsonReader & request) final;
		
        virtual int GetPriority() const { return 500; }
	private:
        void Invoke(ServiceMethod * method, const com::Rpc_Request * request);
    private:
        int mNodeId;
        std::string mMessageBuffer;
        class ProtoRpcComponent *mRpcComponent;
        class CoroutineComponent *mCorComponent;
        class RpcConfigComponent * mPpcConfigComponent;
    };
}