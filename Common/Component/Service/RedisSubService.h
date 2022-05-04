//
// Created by yjz on 2022/1/22.
//

#include"Json/JsonWriter.h"
#include"Component/Component.h"
#include"Method/MethodRegister.h"
#include"Component/RpcService/RemoteServiceComponent.h"
#ifndef SENTRY_REDISSUBSERVICE_H
#define SENTRY_REDISSUBSERVICE_H

namespace Sentry
{
	class RedisSubService : public RemoteServiceComponent,
		public ICallService<com::Rpc::Request, com::Rpc::Response>
	{
	 public:
		RedisSubService() = default;
		virtual ~RedisSubService() = default;
	 public:
		void GetSubMethods(std::vector<std::string>& methods);
	public:
		bool IsStartComplete() final { return true; }
		bool IsStartService() final { return this->mServiceRegister != nullptr; }
	protected:
		bool LateAwake() override;
		virtual bool OnInitService(SubServiceRegister & methodRegister) = 0;
		XCode Send(const std::string &address, std::shared_ptr<Rpc_Request> message) final;
		XCode Send(const std::string &address, std::shared_ptr<Rpc_Response> message) final;
		XCode SendRequest(const std::string &address, std::shared_ptr<com::Rpc::Request> request) final;
		XCode Invoke(const std::string &func, std::shared_ptr<Rpc_Request>, std::shared_ptr<Rpc_Response> response) final;
	 protected:
		bool GetEntityAddress(long long id, std::string &address) final { return false;}
	 public:
		bool LoadService() final;
		void OnAddAddress(const std::string &address) final;
	private:
		class MainRedisComponent * mRedisComponent;
		std::shared_ptr<SubServiceRegister> mServiceRegister;
	};
}

#endif //SENTRY_REDISSUBSERVICE_H
