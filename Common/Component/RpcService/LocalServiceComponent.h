//
// Created by mac on 2022/4/6.
//

#ifndef SERVER_LOCALSERVICECOMPONENT_H
#define SERVER_LOCALSERVICECOMPONENT_H
#include"RemoteServiceComponent.h"
#include"Method/MethodRegister.h"
#include"Component/Lua/LuaScriptComponent.h"

namespace Sentry
{
	class LocalServiceComponent : public RemoteServiceComponent, public IServiceBase
	{
	public:
		LocalServiceComponent() = default;
	protected:
		virtual bool OnInitService(ServiceMethodRegister & methodRegister) = 0;
	public:
		bool DelEntity(long long id, bool publish = false);
		bool AddEntity(long long id, const std::string & address, bool publish = false);
		XCode Invoke(const std::string &func, std::shared_ptr<Rpc_Request>, std::shared_ptr<Rpc_Response> response);
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
	 private:
		class UserSubService * mUserService;
		class RpcClientComponent * mRpcClientComponent;
		std::shared_ptr<ServiceMethodRegister> mMethodRegister;
		std::unordered_map<long long, std::string> mUserAddressMap;
	 protected:
		std::set<std::string> mRemoteAddressList;
	};
}
#endif //SERVER_LOCALSERVICECOMPONENT_H
