#pragma once
#include"RpcTaskComponent.h"
#include"Rpc/Client/Message.h"
#include"Rpc/Async/RpcTaskSource.h"
#include"Rpc/Config/MethodConfig.h"
#include"Log/Common/Logger.h"
namespace acs
{
	class RpcService;

	class LuaRpcService;

	class ServiceMethod;
	class DispatchComponent : public RpcTaskComponent<unsigned int, rpc::Packet>,
							  public IServerRecord, public IAppStop
	{
	 public:
		DispatchComponent();
    public:
		int OnMessage(rpc::Packet * message);
		inline int BuildRpcId() { return this->mNumPool.BuildNumber();}
	private:
		bool LateAwake() final;
		void OnAppStop() final;
		void OnRecord(json::w::Document &document) final;
		void Invoke(const RpcMethodConfig * config, rpc::Packet * message);
	private:
		int OnClient(rpc::Packet * message);
		int OnRequest(rpc::Packet * message);
		int OnBroadcast(rpc::Packet * message);
    private:
		unsigned int mSumCount;
		unsigned int mWaitCount;
		math::NumberPool<int> mNumPool;
		class CoroutineComponent* mTaskComponent;
		class OuterNetComponent* mOuterComponent;
		class RouterComponent * mRouterComponent;
		custom::HashMap<std::string, RpcService *> mRpcServices;
	};
}