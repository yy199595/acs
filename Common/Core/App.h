#pragma once

#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#endif

#include <Define/CommonTypeDef.h>
#include <Global/ServerConfig.h>
#include <Util/TimeHelper.h>
#include <Object/GameObject.h>

#include <Timer/TimerComponent.h>
#include <Coroutine/CoroutineComponent.h>
using namespace std;
using namespace asio::ip;

namespace Sentry
{
	class Manager;

	class ServiceComponent;

	class App : public GameObject
	{
	public:
		App(const std::string srvName, const std::string cfgDir);

		virtual ~App()
		{};

	public:
		ServerConfig &GetConfig()
		{
			return this->mConfig;
		}

		inline float GetDelaTime()
		{
			return this->mDelatime;
		}

		inline long long GetStartTime()
		{
			return this->mStartTime;
		}

		const std::string &GetServerName()
		{
			return this->mServerName;
		}

		inline AsioContext &GetTcpContext()
		{
			return *mTcpContext;
		}

        inline AsioContext &GetHttpContext()
        {
            return *mHttpContext;
        }

		long long GetRunTime()
		{
			return TimeHelper::GetMilTimestamp() - this->mStartTime;
		}

		inline const std::string &GetConfigDir()
		{
			return this->mConfigDir;
		}

		inline bool IsMainThread()
		{
			return std::this_thread::get_id() == this->mMainThreadId;
		}

		inline bool IsTcpThread()
		{
			return std::this_thread::get_id() == this->mTcpThreadId;
		}

        inline bool IsHttpThread()
        {
            return std::this_thread::get_id() == this->mHttpThreadId;
        }

	public:
		static App &Get()
		{
			return *mApp;
		}
	public:	
		inline TimerComponent * GetTimerComponent() { return this->mTimerComponent; }
		inline CoroutineComponent * GetCoroutineComponent() { return this->mCoroutienComponent; }
	private:
		bool LoadComponent();

		bool InitComponent();

		bool StartNetThread();

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
		int LogicMainLoop();

		void TcpThreadLoop();

        void HttpThreadLoop();
	private:
        long long mNextRefreshTime;
		std::thread::id mMainThreadId;
		std::thread::id mTcpThreadId;
        std::thread::id mHttpThreadId;
    private:
        AsioWork * mTcpWork;
        AsioWork * mHttpWork;
        AsioContext * mTcpContext;
        AsioContext * mHttpContext;
        std::thread * mTcpWorkThread;
        std::thread * mHttpWoekThread;
	private:
		bool mIsClose;
		bool mIsInitComplate;
		long long mStartTime;
		ServerConfig mConfig;
		std::string mServerName;
		long long mLastUpdateTime;
		long long mLastSystemTime;
		std::string mConfigDir;

	private:
		float mLogicFps;
		float mDelatime;

	private:
		long long mLogicRunCount;
		long long mSystemRunCount;
		long long mMainLoopStartTime;

	private:
		static App * mApp;
	private:
		TimerComponent * mTimerComponent;
		CoroutineComponent * mCoroutienComponent;
		std::vector<Component *> mSceneComponents;
		std::vector<IFrameUpdate *> mFrameUpdateManagers;
		std::vector<ISystemUpdate *> mSystemUpdateManagers;
		std::vector<ISecondUpdate *> mSecondUpdateManagers;
		std::vector<ILastFrameUpdate *> mLastFrameUpdateManager;
    private:
		std::vector<ITcpContextUpdate *> mTcpUpdateComponent;
        std::vector<IHttpContextUpdate *> mHttpUpdateComponent;

    };
}// namespace Sentry