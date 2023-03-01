#pragma once
#include"Tcp/Asio.h"
#include"Unit/Unit.h"
#include"Config/ServerConfig.h"
#include"Time/TimeHelper.h"
#include"Config/ServerPath.h"
#include"Singleton/Singleton.h"
#include"Config/ServiceConfig.h"
#include"Component/TaskComponent.h"
#include"Component/TimerComponent.h"
#include"Component/LoggerComponent.h"

namespace Sentry
{
	class RpcService;
	class ProtoComponent;
    class App final : public Unit, public Singleton<App>
	{
	 public:
		explicit App();
	 public:
        inline float GetFps() const { return this->mLogicFps; }
		inline LoggerComponent* GetLogger() { return this->mLogComponent; }
		inline Asio::Context & MainThread() { return *this->mMainContext; }
		inline TaskComponent* GetTaskComponent() { return this->mTaskComponent; }
		inline TimerComponent* GetTimerComponent() { return this->mTimerComponent; }
		inline ProtoComponent * GetMsgComponent() { return this->mMessageComponent; }
	 public:
		void Stop();
        int Run(int argc, char ** argv);
        RpcService * GetService(const std::string & name);
        bool OnDelComponent(Component *component) final { return false; }
        inline bool IsMainThread() const { return this->mThreadId == std::this_thread::get_id();}
    private:
#ifdef __OS_WIN__
		void UpdateConsoleTitle();
#endif
		bool LoadComponent();
		void WaitServerStart();
		void StartAllComponent();
		bool InitComponent(Component* component);
	 private:
        int mTickCount;
		float mLogicFps;
        bool mIsStartDone;
        std::thread::id mThreadId;
        const long long mStartTime;
        TaskComponent* mTaskComponent;
		LoggerComponent* mLogComponent;
		TimerComponent* mTimerComponent;
		ProtoComponent * mMessageComponent;
        std::unique_ptr<Asio::Context> mMainContext;
        std::unordered_map<std::string, RpcService*> mServiceMap;
    };
}// namespace Sentry