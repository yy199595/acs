#pragma once


#include"Service.h"
#include"Lua/LuaInclude.h"

namespace Sentry
{
	class LuaScriptComponent;
	class ServiceMethodRegister;
	class LuaService : public Service
	{
	 public:
		LuaService();
		~LuaService() override;
	 protected:
		bool LateAwake() final;
	 public:
        bool Close() final;
        bool Start() final;
        void WaitAllMessageComplete() final;
        int GetWaitMessageCount() const final { return this->mWaitCount; };
        bool IsStartService() final { return this->mMethodRegister != nullptr; }
		XCode Invoke(const std::string& name, std::shared_ptr<Rpc::Data> message) final;
	 private:
        int mWaitCount;
		lua_State* mLuaEnv;
        bool mIsHandlerMessage;
        class LuaScriptComponent* mLuaComponent;
		std::shared_ptr<ServiceMethodRegister> mMethodRegister;
	};
}// namespace Sentry