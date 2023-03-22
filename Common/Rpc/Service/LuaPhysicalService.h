#pragma once


#include"RpcService.h"
#include"Lua/LuaInclude.h"
#include"Json/JsonWriter.h"
#include"Method/MethodRegister.h"
namespace Sentry
{
	class LuaScriptComponent;
	class ServiceMethodRegister;
	class LuaPhysicalService : public RpcService, public IClient, public IServerRecord
	{
	 public:
		LuaPhysicalService();
	 public:
		bool Init() final;
		bool Close() final;
		bool Start() final;
        void WaitAllMessageComplete() final;
		bool LoadFromLua() final { return true; }
		bool IsStartService() final { return true; }
		void OnRecord(Json::Writer &document) final;
		unsigned int GetWaitMessageCount() const final { return this->mWaitCount; };
		int Invoke(const std::string& name, std::shared_ptr<Rpc::Packet> message) final;
	 private:
		void OnLogin(long long userId) final;
		void OnLogout(long long userId) final;
    private:
		bool mIsHandlerMessage;
		unsigned int mSumCount;
		unsigned int mWaitCount;
		unsigned int mUserCount;
		ServiceMethodRegister mMethodRegister;
		class LuaScriptComponent* mLuaComponent;
	};
}// namespace Sentry