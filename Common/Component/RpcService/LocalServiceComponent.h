//
// Created by mac on 2022/4/6.
//

#ifndef SERVER_LOCALSERVICECOMPONENT_H
#define SERVER_LOCALSERVICECOMPONENT_H
#include"CallServiceComponent.h"
#include"Method/MethodRegister.h"
#include"Component/Lua/LuaScriptComponent.h"

namespace Sentry
{
	class LocalServiceComponent : public CallServiceComponent, public IServiceBase
	{
	public:
		LocalServiceComponent() = default;
	protected:
		void Awake();
		bool SubUserEvent();
		virtual bool OnInitService(ServiceMethodRegister & methodRegister) = 0;
		virtual bool OnInitEvent(ServiceEventRegister & methodRegister) { return true;};
	public:
		XCode Invoke(const std::string & eveId, std::shared_ptr<Json::Reader> json);
		XCode Invoke(const std::string &func, std::shared_ptr<com::Rpc::Request>, std::shared_ptr<com::Rpc::Response> response);
	public:
		bool AllotAddress(std::string& address);
		void GetAllAddress(std::list<std::string> & allAddress) const;
	public:
		void OnAddAddress(const std::string &address) final;
		void OnDelAddress(const std::string &address) final;
		bool GetEntityAddress(long long id, std::string& address) final;
		bool IsStartComplete() final { return !this->mAddressList.empty(); }
	public:
		bool LoadEvent();
		bool LoadService() final;
		bool IsStartService() { return this->mMethodRegister != nullptr; }
	private:
		bool OnUserJoin(const Json::Reader & jsonReader);
		bool OnUserExit(const Json::Reader & jsonReader);
	private:
		size_t mIndex;
		std::vector<std::string> mAddressList;
		std::shared_ptr<ServiceEventRegister> mEventRegister;
		std::shared_ptr<ServiceMethodRegister> mMethodRegister;
		std::unordered_map<long long, std::string> mUserAddressMap;
	};
}
#endif //SERVER_LOCALSERVICECOMPONENT_H
