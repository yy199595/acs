#pragma once

#include"Define/CommonTypeDef.h"
#include"Global/ServerConfig.h"
#include"Util/TimeHelper.h"
#include"Entity/Entity.h"
#include"Global/ServerPath.h"
#include"Thread/TaskThread.h"
#include"Global/ServiceConfig.h"
#include"Component/Timer/TimerComponent.h"
#include"Component/Scene/LoggerComponent.h"
#include"Component/Coroutine/TaskComponent.h"
using namespace std;
using namespace asio::ip;

namespace Sentry
{
	class CallServiceComponent;
	class ServiceComponent;
	class MainTaskScheduler;
	class App final : public Entity
	{
	 public:
		explicit App(ServerConfig* config);
		~App() final = default;
	 public:
		static std::shared_ptr<App> Get()
		{
			return mApp;
		}
		const ServerConfig& GetConfig()
		{
			return *mConfig;
		}
		const ServiceConfig& GetServiceConfig()
		{
			return this->mRpcConfig;
		}
		inline MainTaskScheduler& GetTaskScheduler()
		{
			return this->mTaskScheduler;
		}

	 public:

		inline bool IsMainThread()
		{
			return std::this_thread::get_id() == this->mMainThreadId;
		}
		bool StartNewService(const std::string & name);
	 public:
		inline LoggerComponent* GetLogger()
		{
			return this->mLogComponent;
		}
		inline TaskComponent* GetTaskComponent()
		{
			return this->mTaskComponent;
		}
		inline TimerComponent* GetTimerComponent()
		{
			return this->mTimerComponent;
		}
	 private:
		bool LoadComponent();
		void StartAllComponent();
		bool InitComponent(Component* component);
		void OnAddNewService(Component * component);
		bool AddComponentByName(const std::string& name);
	 public:
		int Run();
		void Stop();
	 private:
		void LogicMainLoop();
		bool StartNewComponent();
		void UpdateConsoleTitle();
		void WaitAllServiceStart();
	 private:
		std::thread::id mMainThreadId;
		class MainTaskScheduler mTaskScheduler;
	 private:
		int mFps;
		float mDeltaTime;
		ServiceConfig mRpcConfig;
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
		TaskComponent* mTaskComponent;
		LoggerComponent* mLogComponent;
		TimerComponent* mTimerComponent;
		static std::shared_ptr<App> mApp;
		std::vector<IFrameUpdate*> mFrameUpdateManagers;
		std::vector<ISystemUpdate*> mSystemUpdateManagers;
		std::vector<ISecondUpdate*> mSecondUpdateManagers;
		std::vector<ILastFrameUpdate*> mLastFrameUpdateManager;
	};
}// namespace Sentry