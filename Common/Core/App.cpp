
#include"App.h"
#include"Scene/RpcResponseComponent.h"
#include"Scene/RpcProtoComponent.h"
#include"Component/Scene/RpcComponent.h"
#include"Service/NodeProxyComponent.h"
#include"Scene/RpcRequestComponent.h"
#include"Scene/TaskPoolComponent.h"
#include"Util/DirectoryHelper.h"
#include"Other/ElapsedTimer.h"
using namespace GameKeeper;
using namespace std::chrono;

namespace GameKeeper
{
	App *App::mApp = nullptr;

	App::App(int argc, char ** argv)
		:GameObject(0), mServerPath(argc, argv),
		mStartTime(TimeHelper::GetMilTimestamp()),
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
		this->mServerName = argc == 1 ? "server" : argv[1];
		LogHelper::Init(this->mServerPath.GetLogPath(), this->mServerName);
		this->mConfig = new ServerConfig(this->mServerPath.GetConfigPath() + this->mServerName + ".json");
	}

	void App::OnNewDay()
	{
		spdlog::drop_all();
		LogHelper::Init(this->mServerPath.GetLogPath(), this->mServerName);
	}

	bool App::LoadComponent()
	{
		this->AddComponent<TimerComponent>();
		this->AddComponent<RpcResponseComponent>();
		this->AddComponent<RpcProtoComponent>();
		this->AddComponent<CoroutineComponent>();
		this->AddComponent<RpcComponent>();

		this->AddComponent<NodeProxyComponent>();
		this->AddComponent<RpcRequestComponent>();

		this->mTimerComponent = this->GetComponent<TimerComponent>();
		this->mCorComponent = this->GetComponent<CoroutineComponent>();

		std::vector<std::string> services;
		std::vector<std::string> components;
		if (!mConfig->GetValue("Scene", components))
		{
			GKDebugError("not find field : Managers");
			return false;
		}

		if (!mConfig->GetValue("Service", services))
		{
			GKDebugError("not find field : Service");
			return false;
		}

		for (const std::string & name : components)
		{
			if (!this->AddComponent(name))
			{
				GKDebugFatal("add " << name << " to service failure");
				return false;
			}
		}

		for (const std::string & name : services)
		{
			if (!this->AddComponent(name))
			{
				GKDebugFatal("add " << name << " to scene failure");
				return false;
			}
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
				GKDebugFatal("Init " << component->GetTypeName() << " failure");
				return false;
			}
		}
		this->mCorComponent->StartCoroutine(&App::StartComponent, this);
		return true;
	}

	bool App::InitComponent(Component * component)
	{
		if (!component->IsActive() || !component->Awake())
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
		for (int index = 0; index < this->mSceneComponents.size(); index++)
        {
            ElapsedTimer elapsedTimer;
            Component *component = this->mSceneComponents[index];
            if (component != nullptr)
            {
                component->Start();
                float process = (index + 1) / (float) this->mSceneComponents.size();
                GKDebugInfo("[" << process * 100 << "%]" << " start component "
                                << component->GetTypeName() << " time = " << elapsedTimer.GetMs() << "ms");
            }
        }
		this->mIsInitComplate = true;
		this->mMainLoopStartTime = TimeHelper::GetMilTimestamp();
		GKDebugLog("start all component successful ......");

		for (Component *component : this->mSceneComponents)
		{
            ElapsedTimer elapsedTimer;
			if (auto loadComponent = dynamic_cast<ILoadData *>(component))
            {
                loadComponent->OnLodaData();
                GKDebugLog("load " << component->GetTypeName()
                                   << " data use time = " << elapsedTimer.GetMs() << "ms");
            }
		}
        long long t = TimeHelper::GetMilTimestamp() - this->mStartTime;
		GKDebugLog("=====  start " << this->mServerName << " successful [" << t / 1000.0f << "s] ========");
	}

	int App::Run()
	{
		if (!mConfig->InitConfig() || !this->LoadComponent() || !this->InitComponent())
		{
			return this->Stop();
		}
		mConfig->GetValue("LogicFps", this->mFps);
		this->mLogicUpdateInterval = 1000 / this->mFps;
		this->mStartTime = TimeHelper::GetMilTimestamp();
		this->mSecondTimer = TimeHelper::GetMilTimestamp();
		this->mLastUpdateTime = TimeHelper::GetMilTimestamp();

		return this->mTaskScheduler.Start();
	}

	int App::Stop()
	{
		this->OnDestory();
		this->mIsClose = true;
		spdlog::drop_all();
#ifdef _WIN32
		return getchar();
#endif
		return -1;
	}

	void App::LogicMainLoop()
	{
		this->mStartTimer = TimeHelper::GetMilTimestamp();
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
			this->mStartTimer = mLastUpdateTime = TimeHelper::GetMilTimestamp();
		}

		if (this->mStartTimer - this->mSecondTimer >= 1000)
		{
			for (ISecondUpdate *component : this->mSecondUpdateManagers)
			{
				component->OnSecondUpdate();
			}
			this->UpdateConsoleTitle();
			this->mSecondTimer = TimeHelper::GetMilTimestamp();
		}
	}

    void App::UpdateConsoleTitle()
    {
        long long nowTime = TimeHelper::GetMilTimestamp();
        float seconds = (nowTime - this->mMainLoopStartTime) / 1000.0f;
        this->mLogicFps = this->mLogicRunCount / seconds;
#ifdef _WIN32
        char buffer[100] = {0};
        sprintf_s(buffer, "%s fps:%f", this->mServerName.c_str(), this->mLogicFps);
        SetConsoleTitle(buffer);
#else
        //GKDebugInfo("fps = " << this->mLogicFps);
#endif
    }
}