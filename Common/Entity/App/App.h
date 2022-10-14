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
	class Service;
	class ProtoComponent;
    class App final : public Unit, public Singleton<App>
	{
	 public:
		explicit App();
		~App() final = default;
	 public:
        const std::string & GetConfigPath() const { return this->mConfigPath; }
		inline LoggerComponent* GetLogger() { return this->mLogComponent; }
		inline Asio::Context & GetThread() { return *this->mMainThread; }
		inline TaskComponent* GetTaskComponent() { return this->mTaskComponent; }
		inline TimerComponent* GetTimerComponent() { return this->mTimerComponent; }
		inline ProtoComponent * GetMsgComponent() { return this->mMessageComponent; }
    private:
		bool LoadComponent();
		void StartAllComponent();
		bool InitComponent(Component* component);
	 public:
		void Stop();
        int Run(int argc, char ** argv);
        Service * GetService(const std::string & name);
		bool GetServices(std::vector<Service *> & services);
        inline bool IsMainThread() const { return this->mThreadId == std::this_thread::get_id();}
    private:
		void LogicMainLoop();
		void UpdateConsoleTitle();
		void WaitAllServiceStart();
	 private:
		int mFps;
		float mDeltaTime;
		long long mStartTimer;
		long long mSecondTimer;
		long long mLogicUpdateInterval;
	 private:
		float mLogicFps;
		long long mStartTime;
		long long mLogicRunCount;
		long long mLastUpdateTime;
	 private:
        int mTickCount;
        std::string mConfigPath;
        std::thread::id mThreadId;
        Asio::Context * mMainThread;
        TaskComponent* mTaskComponent;
		LoggerComponent* mLogComponent;
		TimerComponent* mTimerComponent;
		ProtoComponent * mMessageComponent;
		std::vector<IFrameUpdate*> mFrameUpdateManagers;
		std::vector<ISystemUpdate*> mSystemUpdateManagers;
		std::vector<ISecondUpdate*> mSecondUpdateManagers;
        std::unordered_map<std::string, Service*> mSeviceMap;
        std::vector<ILastFrameUpdate*> mLastFrameUpdateManager;
	};
}// namespace Sentry