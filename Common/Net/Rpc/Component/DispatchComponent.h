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
	class DispatchComponent : public RpcTaskComponent<unsigned int, rpc::Message>,
							  public IServerRecord, public IAppStop
	{
	 public:
		DispatchComponent();
    public:
		int OnMessage(rpc::Message * message) noexcept;
		inline int BuildRpcId() { return this->mNumPool.BuildNumber();}
	private:
		bool LateAwake() final;
		void OnAppStop() final;
		void OnRecord(json::w::Document &document) final;
		void Invoke(const RpcMethodConfig * config, rpc::Message * message) noexcept;
	private:
		int OnClient(rpc::Message * message);
		int OnRequest(rpc::Message * message) noexcept;
		int OnBroadcast(rpc::Message * message);
    private:
		unsigned int mSumCount;
		unsigned int mWaitCount;
		math::NumberPool<int> mNumPool;
		class RouterComponent* mRouterComponent;
		class OuterNetComponent* mOuterComponent;
		class CoroutineComponent* mTaskComponent;
		custom::HashMap<std::string, RpcService *> mRpcServices;
	};
}