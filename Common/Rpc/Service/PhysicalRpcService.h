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
		void Start() final;
		void Close() final;
		unsigned int GetWaitMessageCount() const final { return this->mWaitCount; }
		int Invoke(const std::string& func, std::shared_ptr<Msg::Packet> message) final;
	protected:
		bool LoadFromLua() final;
		void WaitAllMessageComplete() final;
		void OnRecord(Json::Writer& document) final;
		inline ServiceMethodRegister& GetMethodRegistry() { return this->mMethodRegister; }
	private:
		unsigned int mSumCount;
		unsigned int mWaitCount;
		ServiceMethodRegister mMethodRegister;
	};
#define BIND_COMMON_RPC_METHOD(func) LOG_CHECK_RET_FALSE(this->GetMethodRegistry().Bind(GET_FUNC_NAME(#func), &func));
#define BIND_ADDRESS_RPC_METHOD(func) LOG_CHECK_RET_FALSE(this->GetMethodRegistry().BindAddress(GET_FUNC_NAME(#func), &func));
}
#endif //SERVER_LOCALSERVICECOMPONENT_H
