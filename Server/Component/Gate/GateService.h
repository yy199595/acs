//
// Created by zmhy0073 on 2021/12/1.
//

#ifndef GAMEKEEPER_GATESERVICE_H
#define GAMEKEEPER_GATESERVICE_H
#include"Component/RpcService/LocalService.h"
namespace Sentry
{
	class GateService final : public LocalRpcService
	{
	 public:
		GateService() = default;
		~GateService() final = default;
	 private:
		XCode Ping(const std::string & address);
		XCode CallClient(long long userId, c2s::rpc::call & request);
		XCode BroadCast(const s2s::broadcast::request & request);
		XCode SaveAddress(long long userId, const s2s::allot::save &request);
		XCode Auth(const std::string & address, const c2s::auth::request & request);
        XCode AllotUser(const com::type::int64 & userId, s2s::allot::response & response);
		XCode QueryAddress(long long userId, const com::type::string &request, com::type::string & response); //查询玩家服务地址
	private:
		bool LateAwake() final;
		bool OnStartService(ServiceMethodRegister & methodRegister) final;
	 private:
		std::string mAddress;
		class TimerComponent* mTimerComponent;
		class UserSyncComponent * mSyncComponent;
		class GateClientComponent* mGateClientComponent;
        std::unordered_map<string, long long> mUserTokens;
	};

}

#endif //GAMEKEEPER_GATESERVICE_H
