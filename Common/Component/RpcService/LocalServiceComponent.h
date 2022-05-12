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
		 void Awake();
		virtual bool OnInitService(ServiceMethodRegister & methodRegister) = 0;
	public:
		bool DelEntity(long long id);
		bool AddEntity(long long id, const std::string & address);
		XCode Invoke(std::shared_ptr<eve::Publish> context);
		XCode Invoke(const std::string &func, std::shared_ptr<Rpc_Request>, std::shared_ptr<Rpc_Response> response);
	public:
		bool AllotAddress(std::string& address);
		bool GetSubEvents(std::list<std::string> & eventIds) const;
		void GetAllAddress(std::list<std::string> & allAddress) const;
	public:
		void OnAddAddress(const std::string &address) final;
		void OnDelAddress(const std::string &address) final;
		bool GetEntityAddress(long long id, std::string& address) final;
		bool IsStartComplete() final { return !this->mAddressList.empty(); }
	public:
		bool LoadService() final;
		bool IsStartService() { return this->mMethodRegister != nullptr; }
	 private:
		std::shared_ptr<ServiceMethodRegister> mMethodRegister;
		std::unordered_map<long long, std::string> mUserAddressMap;
	private:
		size_t mIndex;
		std::vector<std::string> mAddressList;
	};
}
#endif //SERVER_LOCALSERVICECOMPONENT_H
