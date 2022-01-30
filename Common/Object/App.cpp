
#include"App.h"
#include"Service/RpcService.h"
#include"Other/ElapsedTimer.h"
#include"Util/DirectoryHelper.h"
#include"Scene/ServiceMgrComponent.h"
#ifdef __DEBUG__
#include"Telnet/ConsoleComponent.h"
#endif
using namespace Sentry;
using namespace std::chrono;

namespace Sentry
{
	App *App::mApp = nullptr;

	App::App(ServerConfig * config) : Entity(0),
        mConfig(config), mStartTime(Helper::Time::GetMilTimestamp()),
        mTaskScheduler(NewMethodProxy(&App::LogicMainLoop, this))
	{
		mApp = this;
		this->mDelatime = 0;
		this->mIsClose = false;
		this->mLogicRunCount = 0;
		this->mTimerComponent = nullptr;
        this->mMainThreadId = std::this_thread::get_id();
	}

	bool App::AddComponentFormConfig()
    {
        this->AddComponent<LoggerComponent>();
        this->mLogComponent = this->GetComponent<LoggerComponent>();
        LOG_CHECK_RET_FALSE(this->AddComponent<TaskComponent>());
        LOG_CHECK_RET_FALSE(this->AddComponent<TimerComponent>());
        this->mTaskComponent = this->GetComponent<TaskComponent>();
        this->mTimerComponent = this->GetComponent<TimerComponent>();
#ifdef __DEBUG__
        this->AddComponent<ConsoleComponent>();
#endif

        std::vector<std::string> components;
        if (!mConfig->GetValue("component", components)) {
            LOG_ERROR("not find field : component");
            return false;
        }

        if (!mConfig->GetValue("service", components)) {
            LOG_ERROR("not find field : Service");
            return false;
        }

        for (const std::string &name: components)
        {
            if (!this->AddComponent(name)) {
                LOG_FATAL("add ", name, " failure");
                return false;
            }
            //LOG_DEBUG("add new component : " << name);
        }
        return true;
    }

	bool App::InitComponent()
	{
		// 初始化scene组件
        this->GetComponents(this->mSceneComponents);
		for (Component *component : this->mSceneComponents)
		{
			if (!this->InitComponent(component))
			{
                LOG_FATAL("Init ", component->GetTypeName() ," failure");
				return false;
			}
		}
        this->mTaskComponent->Start(&App::StartComponent, this);
		return true;
	}

	bool App::InitComponent(Component * component)
	{
		LOG_CHECK_RET_FALSE(component->IsActive());
		LOG_CHECK_RET_FALSE(component->LateAwake());
		if (auto manager1 = dynamic_cast<IFrameUpdate *>(component)) {
            this->mFrameUpdateManagers.push_back(manager1);
        }
		if (auto manager2 = dynamic_cast<ISystemUpdate *>(component)) {
            this->mSystemUpdateManagers.push_back(manager2);
        }
		if (auto manager3 = dynamic_cast<ISecondUpdate *>(component)) {
            this->mSecondUpdateManagers.push_back(manager3);
        }
		if (auto manager4 = dynamic_cast<ILastFrameUpdate *>(component)) {
            this->mLastFrameUpdateManager.push_back(manager4);
        }
		return true;
	}

	void App::StartComponent()
    {
        for (auto component: this->mSceneComponents)
        {
            auto startComponent = dynamic_cast<IStart *>(component);
            if (startComponent != nullptr)
            {
                LOG_DEBUG("start component ", component->GetTypeName());
                startComponent->OnStart();
            }
        }
        this->mMainLoopStartTime = Helper::Time::GetMilTimestamp();
        LOG_DEBUG("start all component successful ......");

        for (Component *component: this->mSceneComponents)
        {
            ElapsedTimer elapsedTimer;
            if (auto loadComponent = dynamic_cast<ILoadData *>(component))
            {
                loadComponent->OnLoadData();
                LOG_DEBUG("load ", component->GetTypeName(), "data use time = ", elapsedTimer.GetMs(), "ms");
            }
        }

        for (Component *component: this->mSceneComponents)
        {
            component->OnComplete();
        }
        long long t = Helper::Time::GetMilTimestamp() - this->mStartTime;
        LOG_DEBUG("===== start ", this->mServerName, " successful [", t / 1000.0f, "]s =======");
    }

	int App::Run(int argc, char ** argv)
	{
        this->mConfig->GetValue("node_name", this->mServerName);
		if (!this->AddComponentFormConfig())
		{
			return ExitCode::AddError;
		}

		if (!this->InitComponent())
		{
			return ExitCode::InitError;
		}

        this->mFps = 15;
		mConfig->GetValue("fps", this->mFps);
		this->mLogicUpdateInterval = 1000 / this->mFps;
		this->mStartTime = Helper::Time::GetMilTimestamp();
		this->mSecondTimer = Helper::Time::GetMilTimestamp();
		this->mLastUpdateTime = Helper::Time::GetMilTimestamp();

		return this->mTaskScheduler.Start();
	}

	void App::Stop(ExitCode code)
	{
        if(this->mTaskComponent != nullptr)
        {
            this->mTaskComponent->Start([this, code]()
            {
                ElapsedTimer elapsedTimer;
                this->OnDestory();
                this->mIsClose = true;
                this->mTaskScheduler.Stop();
                LOG_WARN("close server successful [", elapsedTimer.GetMs(), "ms]");
                exit((int) code);
            });
        }
	}

	void App::LogicMainLoop()
	{
		this->mStartTimer = Helper::Time::GetMilTimestamp();
		for (ISystemUpdate *component : this->mSystemUpdateManagers)
		{
			component->OnSystemUpdate();
		}

		if (this->mStartTimer - mLastUpdateTime >= this->mLogicUpdateInterval)
		{
			this->mLogicRunCount++;
			for (IFrameUpdate *component : this->mFrameUpdateManagers)
			{
				component->OnFrameUpdate(this->mDelatime);
			}

			if (this->mStartTimer - this->mSecondTimer >= 1000)
			{
				this->UpdateConsoleTitle();
				for (ISecondUpdate* component : this->mSecondUpdateManagers)
				{
					component->OnSecondUpdate();
				}
				this->mSecondTimer = Helper::Time::GetMilTimestamp();
			}

			for (ILastFrameUpdate *component : this->mLastFrameUpdateManager)
			{
				component->OnLastFrameUpdate();
			}
			this->mStartTimer = mLastUpdateTime = Helper::Time::GetMilTimestamp();
		}
	}

    void App::UpdateConsoleTitle()
    {
        long long nowTime = Helper::Time::GetMilTimestamp();
        float seconds = (nowTime - this->mSecondTimer) / 1000.0f;
        this->mLogicFps = (float)this->mLogicRunCount / seconds;
#ifdef _WIN32
        char buffer[100] = {0};
        sprintf_s(buffer, "%s fps:%f", this->mServerName.c_str(), this->mLogicFps);
        SetConsoleTitle(buffer);
#else
        //LOG_INFO("fps = " << this->mLogicFps);
#endif
		this->mLogicRunCount = 0;
    }
}