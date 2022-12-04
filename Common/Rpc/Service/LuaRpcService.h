#pragma once


#include"RpcService.h"
#include"Lua/LuaInclude.h"
#include"Method/EventMethod.h"
namespace Sentry
{
	class LuaScriptComponent;
	class ServiceMethodRegister;
	class LuaRpcService : public RpcService
	{
	 public:
		LuaRpcService();
	 protected:
		bool LateAwake() final;
	 public:
        bool Close() final;
        bool Start() final;
		bool LoadFromLua() final;
        void WaitAllMessageComplete() final;
        int GetWaitMessageCount() const final { return this->mWaitCount; };
        bool IsStartService() final { return this->mMethodRegister != nullptr; }
		XCode Invoke(const std::string& name, std::shared_ptr<Rpc::Packet> message) final;

    private:
    private:
        int mWaitCount;
        bool mIsHandlerMessage;
        class LuaScriptComponent* mLuaComponent;
		std::shared_ptr<ServiceMethodRegister> mMethodRegister;
	};
}// namespace Sentry