#pragma once


#include"RpcService.h"
#include"Lua/LuaInclude.h"
#include"Method/EventMethod.h"
namespace Sentry
{
	class LuaScriptComponent;
	class ServiceMethodRegister;
	class LuaPhysicalService : public RpcService, public IClient
	{
	 public:
		LuaPhysicalService();
	 protected:
		bool LateAwake() final;
	 public:
        bool Close() final;
        bool Start() final;
		bool LoadFromLua() final;
        void WaitAllMessageComplete() final;
		bool IsStartService() final { return true; }
		unsigned int GetWaitMessageCount() const final { return this->mWaitCount; };
		int Invoke(const std::string& name, std::shared_ptr<Rpc::Packet> message) final;
	 private:
		void OnLogin(long long userId) final;
		void OnLogout(long long userId) final;
    private:
        int mWaitCount;
        bool mIsHandlerMessage;
        class LuaScriptComponent* mLuaComponent;
		std::unique_ptr<ServiceMethodRegister> mMethodRegister;
	};
}// namespace Sentry