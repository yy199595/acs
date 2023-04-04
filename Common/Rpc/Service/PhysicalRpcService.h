//
// Created by mac on 2022/4/6.
//

#ifndef SERVER_LOCALSERVICECOMPONENT_H
#define SERVER_LOCALSERVICECOMPONENT_H
#include"RpcService.h"
#include"Rpc/Method/MethodRegister.h"
namespace Tendo
{
	//ʵ����� �ܴ����߼�
	class PhysicalRpcService : public RpcService, public IServerRecord
	{
	public:
		PhysicalRpcService();
	public:
		bool Init() final;
		bool Start() final;
		bool Close() final;
		bool IsStartService() final{ return true; }
		unsigned int GetWaitMessageCount() const final { return this->mWaitCount; }
		int Invoke(const std::string& func, std::shared_ptr<Rpc::Packet> message) final;
	protected:
		bool LoadFromLua() final;
		virtual void OnClose(){ };
		virtual bool OnInit() = 0;
		virtual bool OnStart()  { return true; }
		void WaitAllMessageComplete() final;
		void OnRecord(Json::Writer& document) final;
		inline ServiceMethodRegister& GetMethodRegistry() { return this->mMethodRegister; }
	private:
		bool mIsHandle;
		unsigned int mSumCount;
		unsigned int mWaitCount;
		ServiceMethodRegister mMethodRegister;
	};
#define BIND_COMMON_RPC_METHOD(func) LOG_CHECK_RET_FALSE(this->GetMethodRegistry().Bind(GET_FUNC_NAME(#func), &func));
#define BIND_ADDRESS_RPC_METHOD(func) LOG_CHECK_RET_FALSE(this->GetMethodRegistry().BindAddress(GET_FUNC_NAME(#func), &func));
}
#endif //SERVER_LOCALSERVICECOMPONENT_H
