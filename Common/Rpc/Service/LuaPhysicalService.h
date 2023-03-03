#pragma once


#include"RpcService.h"
#include"Lua/LuaInclude.h"
#include"Method/EventMethod.h"
namespace Sentry
{
	class LuaScriptComponent;
	class ServiceMethodRegister;
	class LuaPhysicalService : public RpcService
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
		int GetWaitMessageCount() const final { return this->mWaitCount; };
		XCode Invoke(const std::string& name, std::shared_ptr<Rpc::Packet> message) final;

    private:
    private:
        int mWaitCount;
        bool mIsHandlerMessage;
        class LuaScriptComponent* mLuaComponent;
		std::unique_ptr<ServiceMethodRegister> mMethodRegister;
	};
}// namespace Sentry