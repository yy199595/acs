
#include "App.h"

#include <Util/FileHelper.h>
#include <Scene/SceneActionComponent.h>
#include <Scene/SceneProtocolComponent.h>
#include <Scene/SceneSessionComponent.h>
#include <Service/ServiceNodeComponent.h>
#include <Service/ServiceMgrComponent.h>

using namespace Sentry;
using namespace std::chrono;

namespace Sentry
{
    App *App::mApp = nullptr;

    App::App(const std::string srvName, const std::string configPath)
            : mStartTime(TimeHelper::GetMilTimestamp()), mConfig(configPath),
		Scene(0), Service(1)
    {       
		mApp = this;
        this->mDelatime = 0;
        this->mLogicTime = 0;
		this->mIsClose = false;
        this->mServerName = srvName;
        this->mLogicRunCount = 0;
        this->mSystemRunCount = 0;
		this->mIsInitComplate = false;
        this->mNetWorkThread = nullptr;
        this->mSrvConfigDirectory = configPath;
        this->mLogHelper = new LogHelper("./Logs", srvName);
    }

    bool App::LoadComponent()
    {
		this->Scene.AddComponent<TimerComponent>();
		this->Scene.AddComponent<SceneActionComponent>();
		this->Scene.AddComponent<SceneProtocolComponent>();
		this->Scene.AddComponent<CoroutineComponent>();
		this->Scene.AddComponent<SceneSessionComponent>();

		this->Service.AddComponent<ServiceNodeComponent>();
		this->Service.AddComponent<ServiceMgrComponent>();

		this->mTimerComponent = this->Scene.GetComponent<TimerComponent>();
		this->mCoroutienComponent = this->Scene.GetComponent<CoroutineComponent>();



		std::vector<std::string> services;
        std::vector<std::string> managers;
        if (!mConfig.GetValue("Scene", managers))
        {
            SayNoDebugError("not find field : Managers");
            return false;
        }

		if (!mConfig.GetValue("Service", services))
		{
			SayNoDebugError("not find field : Service");
			return false;
		}

        for (size_t index = 0; index < services.size(); index++)
        {
            const std::string &name = services[index];
			if (this->Service.AddComponent(name) == false)
			{
				SayNoDebugFatal("add " << name << " to scene failure");
				return false;
			}
        }

		for (size_t index = 0; index < managers.size(); index++)
		{
			const std::string &name = managers[index];
			if (this->Scene.AddComponent(name) == false)
			{
				SayNoDebugFatal("add " << name << " to service failure");
				return false;
			}
		}
        return true;
    }

	bool App::InitComponent()
	{

		// 初始化scene组件
		std::vector<Component *> components;
		this->Scene.GetComponents(components);
		std::sort(components.begin(), components.end(),
			[](Component * m1, Component * m2)->bool
		{
			return m1->GetPriority() < m2->GetPriority();
		});

		for(Component * component : components)
		{
			SayNoAssertRetFalse_F(this->InitComponent(component));
			this->mAllComponents.push_back(component);
		}
		components.clear();

		//初始化servie组件
		this->Service.GetComponents(components);
		std::sort(components.begin(), components.end(),
			[](Component * m1, Component * m2)->bool
		{
			return m1->GetPriority() < m2->GetPriority();
		});

		for(Component * component : components)
		{
			SayNoAssertRetFalse_F(this->InitComponent(component));
			this->mAllComponents.push_back(component);
		}

		SayNoAssertRetFalse_F(this->StartNetThread());
		CoroutineComponent * pCoroutineManager = this->Scene.GetComponent<CoroutineComponent>();
		pCoroutineManager->StartCoroutine(&App::StartComponent, this);
		return true;
	}

	bool App::InitComponent(Component * component)
	{
		if (component->IsActive() == false || component->Awake() == false)
		{
			SayNoDebugError("init " << component->GetTypeName() << " fail");
			return false;
		}

		IFrameUpdate *manager1 = dynamic_cast<IFrameUpdate *>(component);
		ISystemUpdate *manager2 = dynamic_cast<ISystemUpdate *>(component);
		ISecondUpdate *manager3 = dynamic_cast<ISecondUpdate *>(component);
		INetSystemUpdate *manager4 = dynamic_cast<INetSystemUpdate *>(component);
		if (manager1 != nullptr)
		{
			this->mFrameUpdateManagers.push_back(manager1);			
		}
		if (manager2 != nullptr)
		{
			this->mSystemUpdateManagers.push_back(manager2);
		}
		if (manager3 != nullptr)
		{
			this->mSecondUpdateManagers.push_back(manager3);
		}
		if (manager4 != nullptr)
		{
			this->mNetSystemUpdateManagers.push_back(manager4);
		}
		return true;
	}

	bool App::StartNetThread()
	{
		this->mNetWorkThread = new std::thread(std::bind(BIND_THIS_ACTION_0(App::NetworkLoop)));
		if (this->mNetWorkThread != nullptr)
		{
			this->mMainThreadId = std::this_thread::get_id();
			this->mNetWorkThreadId = this->mNetWorkThread->get_id();
			SayNoDebugInfo("main thread : [" 
				<< this->mMainThreadId << "]  " << "net thread : [" << this->mNetWorkThreadId << "]");
			return true;
		}
		return false;
	}

	void App::StartComponent()
	{
		for (size_t index = 0; index < this->mAllComponents.size(); index++)
		{
			Component * component = this->mAllComponents[index];
			if (component != nullptr)
			{
				float process = index / (float)this->mAllComponents.size();
				SayNoDebugInfo("[" << process * 100 << "%]" << " start component " << component->GetTypeName());

				component->Start();
			}
		}		
		this->mIsInitComplate = true;
		long long t = TimeHelper::GetMilTimestamp() - this->mStartTime;
		SayNoDebugLog("=====  start " << this->mServerName << " successful ["<< t / 1000.0f <<"s] ========");
	}

	int App::Run()
    {
        if (!mConfig.InitConfig() || !this->LoadComponent() || !this->InitComponent())
        {
			return this->Stop();
        }
        return this->LogicMainLoop();
    }

    int App::Stop()
    {
        this->mIsClose = true;
		this->Scene.OnDestory();
		this->Service.OnDestory();
        this->mLogHelper->DropLog();
#ifdef _WIN32
        return getchar();
#endif
        return -1;
    }

    float App::GetMeanFps()
    {
        return 0;
    }

    int App::LogicMainLoop()
    {
		int logicFps = 30;
		mConfig.GetValue("LogicFps", logicFps);
        long long startTimer = TimeHelper::GetMilTimestamp();
        long long secondTimer = TimeHelper::GetMilTimestamp();
        this->mLastUpdateTime = TimeHelper::GetMilTimestamp();
        this->mMainLoopStartTime = TimeHelper::GetMilTimestamp();   
       

        std::chrono::milliseconds time(1);
        const long long LogicUpdateInterval = 1000 / logicFps;
        while (!this->mIsClose)
        {
            std::this_thread::sleep_for(time);
            startTimer = TimeHelper::GetMilTimestamp();
            for (size_t index = 0; index < this->mSystemUpdateManagers.size(); index++)
            {
                this->mSystemUpdateManagers[index]->OnSystemUpdate();
            }
			if (this->mIsInitComplate)
			{
				if (startTimer - mLastUpdateTime >= LogicUpdateInterval)
				{
					this->mLogicRunCount++;
					for (size_t index = 0; index < this->mFrameUpdateManagers.size(); index++)
					{
						this->mFrameUpdateManagers[index]->OnFrameUpdate(this->mDelatime);
					}

					for (size_t index = 0; index < this->mLastFrameUpdateManager.size(); index++)
					{
						this->mLastFrameUpdateManager[index]->OnLastFrameUpdate();
					}

					startTimer = mLastUpdateTime = TimeHelper::GetMilTimestamp();
				}

				if (startTimer - secondTimer >= 1000)
				{
					for (size_t index = 0; index < this->mSecondUpdateManagers.size(); index++)
					{
						this->mSecondUpdateManagers[index]->OnSecondUpdate();
					}
					this->UpdateConsoleTitle();
					secondTimer = TimeHelper::GetMilTimestamp();
				}
			}
        }
        return this->Stop();
    }

    void App::NetworkLoop()
    {
        std::chrono::milliseconds time(1);
        while (this->mIsClose == false)
        {
            mAsioContext.poll();
            std::this_thread::sleep_for(time);
            for (size_t index = 0; index < this->mNetSystemUpdateManagers.size(); index++)
            {
                INetSystemUpdate *manager = this->mNetSystemUpdateManagers[index];
                manager->OnNetSystemUpdate(this->mAsioContext);
            }
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
#endif
    }
}// namespace Sentry