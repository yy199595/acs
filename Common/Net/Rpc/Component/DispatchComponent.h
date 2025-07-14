#pragma once
#include"RpcComponent.h"
#include"Rpc/Common/Message.h"
#include"Rpc/Async/RpcTaskSource.h"
#include"Rpc/Config/MethodConfig.h"
#include"Log/Common/Logger.h"
namespace acs
{
	class RpcService;

	class LuaRpcService;

	class ServiceMethod;
	class DispatchComponent : public RpcComponent<rpc::Message>,
							  public IServerRecord, public IAppStop
	{
	 public:
		DispatchComponent();
    public:
		int OnMessage(std::unique_ptr<rpc::Message> & message) noexcept;
	private:
		bool LateAwake() final;
		void OnAppStop() final;
		void OnRecord(json::w::Document &document) final;
		void Invoke(const RpcMethodConfig * config, std::unique_ptr<rpc::Message> & message) noexcept;
	private:
		int OnClient(std::unique_ptr<rpc::Message> & message);
		int OnRequest(std::unique_ptr<rpc::Message> & message) noexcept;
		int OnBroadcast(std::unique_ptr<rpc::Message> & message);
    private:
		unsigned int mSumCount;
		unsigned int mWaitCount;
		rpc::IOuterSender* mGateSender;
		class RouterComponent* mRouter;
		class CoroutineComponent* mCoroutine;
		custom::HashMap<std::string, RpcService *> mRpcServices;
	};
}