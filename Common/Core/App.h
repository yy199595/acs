#pragma once

#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#endif

#include <Define/CommonTypeDef.h>
#include <Global/ServerConfig.h>
#include <Util/TimeHelper.h>
#include <Object/GameObject.h>
#include <Global/ServerPath.h>
#include <Timer/TimerComponent.h>
#include <Thread/TaskThread.h>
#include <Coroutine/CoroutineComponent.h>
using namespace std;
using namespace asio::ip;


namespace GameKeeper
{
	class Manager;

	class ServiceComponent;
	class MainTaskScheduler;

	class App : public GameObject
	{
	public:
		App(int argc, char ** argv);

		virtual ~App() final = default;

	public:
		const ServerConfig &GetConfig()
		{
			return *mConfig;
		}

		inline float GetDelaTime() const
		{
			return this->mDelatime;
		}

		inline long long GetStartTime() const
		{
			return this->mStartTime;
		}

		const std::string &GetServerName()
		{
			return this->mServerName;
		}

		inline MainTaskScheduler & GetTaskScheduler() { return this->mTaskScheduler; }

		const std::string & GetWorkPath() { return this->mServerPath.GetWorkPath(); }

		const std::string & GetConfigPath() { return this->mServerPath.GetConfigPath(); }

		const std::string & GetDownloadPath() { return this->mServerPath.GetDownloadPath(); }

		inline bool IsMainThread()
		{
			return std::this_thread::get_id() == this->mMainThreadId;
		}

	public:
		static App &Get()
		{
			return *mApp;
		}
	public:	
		inline TimerComponent * GetTimerComponent() { return this->mTimerComponent; }
		inline CoroutineComponent * GetCorComponent() { return this->mCorComponent; }
	private:
		bool LoadComponent();

		bool InitComponent();

		bool InitComponent(Component * component);

		void StartComponent();

        void OnZeroRefresh();
	public:
		int Run();

		int Stop();

		void Hotfix();

	private:
		void UpdateConsoleTitle();
	private:
		void LogicMainLoop();
	private:
        ServerPath mServerPath;
        long long mNextRefreshTime;
		std::thread::id mMainThreadId;
		class MainTaskScheduler mTaskScheduler;
	private:
		int mFps;
		long long mStartTimer;
		long long mSecondTimer;
		long long mLogicUpdateInterval;
	private:
		bool mIsClose;
		bool mIsInitComplate;
		long long mStartTime;
		ServerConfig * mConfig;
		std::string mServerName;
		long long mLastUpdateTime;
	private:
		float mLogicFps;
		float mDelatime;

	private:
		long long mLogicRunCount;
		long long mMainLoopStartTime;

	private:
		static App * mApp;
	private:
		TimerComponent * mTimerComponent;
		CoroutineComponent * mCorComponent;
		std::vector<Component *> mSceneComponents;
		std::vector<IFrameUpdate *> mFrameUpdateManagers;
		std::vector<ISystemUpdate *> mSystemUpdateManagers;
		std::vector<ISecondUpdate *> mSecondUpdateManagers;
		std::vector<ILastFrameUpdate *> mLastFrameUpdateManager;
    };
}// namespace GameKeeper