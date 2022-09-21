#pragma once

#include"Unit/Unit.h"
#include"Define/CommonTypeDef.h"
#include"Config/ServerConfig.h"
#include"Time/TimeHelper.h"
#include"Config/ServerPath.h"
#include"Config/ServiceConfig.h"
#include"Component/TaskComponent.h"
#include"Component/TimerComponent.h"
#include"Component/LoggerComponent.h"
using namespace asio::ip;

namespace Sentry
{
	class Service;
	class ProtoComponent;
	class App final : public Unit
	{
	 public:
		explicit App(ServerConfig* config);
		~App() final = default;
	 public:
		bool StartNewService(const std::string & name);
		static std::shared_ptr<App> Get() { return mApp; }
		const ServerConfig& GetConfig() { return *mConfig; }
		inline LoggerComponent* GetLogger() { return this->mLogComponent; }
		inline asio::io_service& GetThread() { return *this->mMainThread; }
		inline TaskComponent* GetTaskComponent() { return this->mTaskComponent; }
		inline TimerComponent* GetTimerComponent() { return this->mTimerComponent; }
		inline ProtoComponent * GetMsgComponent() { return this->mMessageComponent; }
	 private:
		bool LoadComponent();
		void StartAllComponent();
		bool InitComponent(Component* component);
		void OnAddNewService(Component * component);
	 public:
		int Run();
		void Stop();
		Service * GetService(const std::string & name);
		bool GetServices(std::vector<Service *> & services);
        inline bool IsMainThread() const { return this->mThreadId == std::this_thread::get_id();}
    private:
		void LogicMainLoop();
		bool StartNewComponent();
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
		ServerConfig* mConfig;
		std::string mServerName;
		long long mLogicRunCount;
		long long mLastUpdateTime;
	 private:
        int mTickCount;
        std::thread::id mThreadId;
        asio::io_service * mMainThread;
        TaskComponent* mTaskComponent;
		LoggerComponent* mLogComponent;
		TimerComponent* mTimerComponent;
		static std::shared_ptr<App> mApp;
		ProtoComponent * mMessageComponent;
		std::vector<IFrameUpdate*> mFrameUpdateManagers;
		std::vector<ISystemUpdate*> mSystemUpdateManagers;
		std::vector<ISecondUpdate*> mSecondUpdateManagers;
        std::unordered_map<std::string, Service*> mSeviceMap;
        std::vector<ILastFrameUpdate*> mLastFrameUpdateManager;
	};
}// namespace Sentry