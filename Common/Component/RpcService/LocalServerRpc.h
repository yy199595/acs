//
// Created by mac on 2022/4/6.
//

#ifndef SERVER_LOCALSERVERRPC_H
#define SERVER_LOCALSERVERRPC_H
#include"RpcServiceBase.h"
#include"Method/MethodRegister.h"
#include"Component/Lua/LuaScriptComponent.h"

namespace Sentry
{
	class LocalServerRpc : public RpcServiceBase
	{
	public:
		LocalServerRpc() = default;
	protected:
		virtual bool OnInitService(ServiceMethodRegister & methodRegister) = 0;
		bool OnCallLocal(std::shared_ptr<com::Rpc::Request> request, std::shared_ptr<com::Rpc::Response>) override { return false;}
	public:
		bool DelEntity(long long id, bool publish = false);
		bool AddEntity(long long id, const std::string & address, bool publish = false);
	 public:
		bool AllotAddress(std::string& address) const;
		void OnAddAddress(const std::string &address) final;
		void OnDelAddress(const std::string &address) final;
		bool GetEntityAddress(long long id, std::string& address) final;
		bool IsStartComplete() final { return !this->mRemoteAddressList.empty(); }
	public:
		bool LoadService() final;
		bool LateAwake() override;
		bool IsStartService() { return this->mMethodRegister != nullptr; }
		std::shared_ptr<com::Rpc_Response> Invoke(const std::string& method, std::shared_ptr<com::Rpc_Request> request);
	 private:
		class UserSubService * mUserService;
		class RedisComponent * mRedisComponent;
		std::set<std::string> mRemoteAddressList;
		std::shared_ptr<ServiceMethodRegister> mMethodRegister;
		std::unordered_map<long long, std::string> mUserAddressMap;
	};
}
#endif //SERVER_LOCALSERVERRPC_H
