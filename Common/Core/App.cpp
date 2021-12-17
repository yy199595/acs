
#include"App.h"
#include"Service/NodeProxyComponent.h"
#include"Scene/ThreadPoolComponent.h"
#include"Util/DirectoryHelper.h"
#include"Other/ElapsedTimer.h"
using namespace GameKeeper;
using namespace std::chrono;

namespace GameKeeper
{
	App *App::mApp = nullptr;

	App::App() :GameObject(0), mServerPath(nullptr),
		mStartTime(Helper::Time::GetMilTimestamp()),
		mTaskScheduler(NewMethodProxy(&App::LogicMainLoop, this))
	{
		mApp = this;
		this->mDelatime = 0;
		this->mIsClose = false;
		this->mConfig = nullptr;
		this->mLogicRunCount = 0;
		this->mIsInitComplate = false;
		this->mTimerComponent = nullptr;
        this->mMainThreadId = std::this_thread::get_id();
	}

	bool App::AddComponentFormConfig()
	{
        this->AddComponent<TaskComponent>();
		this->AddComponent<TimerComponent>();
		this->AddComponent<LoggerComponent>();

        this->mTaskComponent = this->GetComponent<TaskComponent>();
        this->mLogComponent = this->GetComponent<LoggerComponent>();
		this->mTimerComponent = this->GetComponent<TimerComponent>();

		std::vector<std::string> components;
		if (!mConfig->GetValue("Scene", components))
		{
			LOG_ERROR("not find field : Scene");
			return false;
		}

		if (!mConfig->GetValue("Service", components))
		{
			LOG_ERROR("not find field : Service");
			return false;
		}

		for (const std::string & name : components)
		{
			if (!this->AddComponent(name))
			{
				LOG_FATAL("add " << name << " to service failure");
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
		std::sort(mSceneComponents.begin(), mSceneComponents.end(),
			[](Component *m1, Component *m2) -> bool
		{
			return m1->GetPriority() < m2->GetPriority();
		});

		for (Component *component : mSceneComponents)
		{
			if (!this->InitComponent(component))
			{
				LOG_FATAL("Init " << component->GetTypeName() << " failure");
				return false;
			}
		}
        this->mTaskComponent->Start(&App::StartComponent, this);
		return true;
	}

	bool App::InitComponent(Component * component)
	{
		if (!component->IsActive() || !component->LateAwake())
		{
			return false;
		}

		if (auto manager1 = dynamic_cast<IFrameUpdate *>(component))
		{
			this->mFrameUpdateManagers.push_back(manager1);
		}
		if (auto manager2 = dynamic_cast<ISystemUpdate *>(component))
		{
			this->mSystemUpdateManagers.push_back(manager2);
		}
		if (auto manager3 = dynamic_cast<ISecondUpdate *>(component))
		{
			this->mSecondUpdateManagers.push_back(manager3);
		}

		if (auto manager4 = dynamic_cast<ILastFrameUpdate *>(component))
		{
			this->mLastFrameUpdateManager.push_back(manager4);
		}
		return true;
	}

	void App::StartComponent()
	{
		for (auto component : this->mSceneComponents)
        {
            ElapsedTimer elapsedTimer;
            auto startComponent = dynamic_cast<IStart *>(component);
            if (startComponent != nullptr)
            {
                startComponent->OnStart();
                LOG_INFO(" start component " << component->GetTypeName()
                                             << " use time = " << elapsedTimer.GetMs() << "ms");
                //LOG_DEBUG("start " << component->GetTypeName() << " use time " << elapsedTimer.GetMs() << "ms");
            }
        }
		this->mIsInitComplate = true;
		this->mMainLoopStartTime = Helper::Time::GetMilTimestamp();
		LOG_DEBUG("start all component successful ......");

		for (Component *component : this->mSceneComponents)
		{
            ElapsedTimer elapsedTimer;
			if (auto loadComponent = dynamic_cast<ILoadData *>(component))
            {
                loadComponent->OnLoadData();
                LOG_DEBUG("load " << component->GetTypeName()
                                  << " data use time = " << elapsedTimer.GetMs() << "ms");
            }
		}


        long long t = Helper::Time::GetMilTimestamp() - this->mStartTime;
		LOG_DEBUG("=====  start " << this->mServerName << " successful [" << t / 1000.0f << "s] ========");
	}

	int App::Run(int argc, char ** argv)
	{
		std::string config(argv[1]);
		this->mConfig = new ServerConfig(config);
		this->mServerPath = new ServerPath(argc, argv);		

		if (!mConfig->InitConfig())
		{			
			return this->Stop(ExitCode::ConfigError);
		}

		if (!this->AddComponentFormConfig())
		{
			return this->Stop(ExitCode::AddError);
		}

		if (!this->InitComponent())
		{
			return this->Stop(ExitCode::InitError);
		}

        this->mFps = 15;
		mConfig->GetValue("Fps", this->mFps);
		this->mLogicUpdateInterval = 1000 / this->mFps;
		this->mStartTime = Helper::Time::GetMilTimestamp();
		this->mSecondTimer = Helper::Time::GetMilTimestamp();
		this->mLastUpdateTime = Helper::Time::GetMilTimestamp();

		return this->mTaskScheduler.Start();
	}

	int App::Stop(ExitCode code)
	{
		this->OnDestory();
		this->mIsClose = true;
#ifdef _WIN32
		return getchar();
#endif
		return (int)code;
	}

	void App::LogicMainLoop()
	{
		this->mStartTimer = Helper::Time::GetMilTimestamp();
		for (ISystemUpdate *component : this->mSystemUpdateManagers)
		{
			component->OnSystemUpdate();
		}

		if (!this->mIsInitComplate)
		{
			return;
		}
		if (this->mStartTimer - mLastUpdateTime >= this->mLogicUpdateInterval)
		{
			this->mLogicRunCount++;
			for (IFrameUpdate *component : this->mFrameUpdateManagers)
			{
				component->OnFrameUpdate(this->mDelatime);
			}

			for (ILastFrameUpdate *component : this->mLastFrameUpdateManager)
			{
				component->OnLastFrameUpdate();
			}
			this->mStartTimer = mLastUpdateTime = Helper::Time::GetMilTimestamp();
		}

		if (this->mStartTimer - this->mSecondTimer >= 1000)
		{
			for (ISecondUpdate *component : this->mSecondUpdateManagers)
			{
				component->OnSecondUpdate();
			}
			this->UpdateConsoleTitle();
			this->mSecondTimer = Helper::Time::GetMilTimestamp();
		}
	}

    void App::UpdateConsoleTitle()
    {
        long long nowTime = Helper::Time::GetMilTimestamp();
        long long seconds = (nowTime - this->mMainLoopStartTime) / 1000;
        this->mLogicFps = this->mLogicRunCount / (float)seconds;
#ifdef _WIN32
        char buffer[100] = {0};
        sprintf_s(buffer, "%s fps:%f", this->mServerName.c_str(), this->mLogicFps);
        SetConsoleTitle(buffer);
#else
        //LOG_INFO("fps = " << this->mLogicFps);
#endif
    }
}