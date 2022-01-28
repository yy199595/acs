#pragma once

#include"Define/CommonTypeDef.h"
#include"Global/ServerConfig.h"
#include"Util/TimeHelper.h"
#include"Object/Entity.h"
#include"Global/ServerPath.h"
#include"Thread/TaskThread.h"
#include"Timer/TimerComponent.h"
#include"Scene/LoggerComponent.h"
#include"Coroutine/TaskComponent.h"
using namespace std;
using namespace asio::ip;


namespace Sentry
{
	enum ExitCode
	{
		Exit,
		AddError,
		InitError,
		StartError,
		ConfigError
	};
}

namespace Sentry
{
	class Manager;

	class RpcService;
	class MainTaskScheduler;
	class App : public Entity
	{
	public:
		explicit App(ServerConfig * config);
		~App() final = default;
	public:
        static App &Get() { return *mApp; }
        const ServerConfig &GetConfig() {return *mConfig; }
		inline float GetDelaTime() const { return this->mDelatime;}
		inline MainTaskScheduler & GetTaskScheduler() { return this->mTaskScheduler; }
		inline bool IsMainThread() { return std::this_thread::get_id() == this->mMainThreadId; }
	public:
        template<typename T>
        void GetTypeComponents(std::list<T *> & components);
		inline LoggerComponent * GetLogger() { return this->mLogComponent; }
        inline TaskComponent * GetTaskComponent() { return this->mTaskComponent; }
        inline TimerComponent * GetTimerComponent() { return this->mTimerComponent; }
	private:
		bool InitComponent();
        void StartComponent();
        bool AddComponentFormConfig();
		bool InitComponent(Component * component);
	public:
		void Stop(ExitCode code);
		int Run(int argc, char ** argv);
	private:
		void LogicMainLoop();
        void UpdateConsoleTitle();
    private:
		std::thread::id mMainThreadId;
		class MainTaskScheduler mTaskScheduler;
	private:
		int mFps;
		long long mStartTimer;
		long long mSecondTimer;
        long long mMainLoopStartTime;
		long long mLogicUpdateInterval;
	private:
		bool mIsClose;
        float mLogicFps;
        float mDelatime;
		long long mStartTime;
		ServerConfig * mConfig;
		std::string mServerName;
        long long mLogicRunCount;
        long long mLastUpdateTime;
	private:
        static App * mApp;
        TaskComponent * mTaskComponent;
        LoggerComponent * mLogComponent;
		TimerComponent * mTimerComponent;
		std::vector<Component *> mSceneComponents;
		std::vector<IFrameUpdate *> mFrameUpdateManagers;
		std::vector<ISystemUpdate *> mSystemUpdateManagers;
		std::vector<ISecondUpdate *> mSecondUpdateManagers;
		std::vector<ILastFrameUpdate *> mLastFrameUpdateManager;
    };

    template<typename T>
    void App::GetTypeComponents(std::list<T *> &components)
    {
        for (Component *component: this->mSceneComponents)
        {
            T *typeComponent = dynamic_cast<T *>(component);
            if (typeComponent != nullptr)
            {
                components.push_back(typeComponent);
            }
        }
    }
}// namespace Sentry