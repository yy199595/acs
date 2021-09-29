
#include "App.h"

#include <Util/FileHelper.h>
#include <Scene/ActionComponent.h>
#include <Scene/ProtocolComponent.h>
#include <Scene/NetSessionComponent.h>
#include <Service/ServiceNodeComponent.h>
#include <Service/ServiceMgrComponent.h>

using namespace Sentry;
using namespace std::chrono;

namespace Sentry
{
    App *App::mApp = nullptr;

	App::App(const std::string srvName, const std::string cfgDir)
		: mStartTime(TimeHelper::GetMilTimestamp()),
		mConfig(cfgDir + srvName + ".json"), Scene(1)
	{
		mApp = this;
		this->mDelatime = 0;
		this->mIsClose = false;
		this->mServerName = srvName;
		this->mLogicRunCount = 0;
		this->mSystemRunCount = 0;
		this->mIsInitComplate = false;
		this->mNetWorkThread = nullptr;
		this->mConfigDir = cfgDir;
		this->mAsioContext = new AsioContext(1);
		this->mAsioWork = new AsioWork(*mAsioContext);
		LogHelper::Init("./Logs", srvName);
	}

    bool App::LoadComponent()
    {
		this->Scene.AddComponent<TimerComponent>();
		this->Scene.AddComponent<ActionComponent>();
		this->Scene.AddComponent<ProtocolComponent>();
		this->Scene.AddComponent<CoroutineComponent>();
		this->Scene.AddComponent<NetSessionComponent>();
		this->Scene.AddComponent<NetProxyComponent>();

		this->Scene.AddComponent<ServiceNodeComponent>();
		this->Scene.AddComponent<ServiceMgrComponent>();

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
			if (!this->Scene.AddComponent(name))
			{
				SayNoDebugFatal("add " << name << " to scene failure");
				return false;
			}
        }

		for (size_t index = 0; index < managers.size(); index++)
		{
			const std::string &name = managers[index];
			if (!this->Scene.AddComponent(name))
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
		this->Scene.GetComponents(this->mSceneComponents);
		std::sort(mSceneComponents.begin(), mSceneComponents.end(),
			[](Component * m1, Component * m2)->bool
		{
			return m1->GetPriority() < m2->GetPriority();
		});

		for(Component * component : mSceneComponents)
		{
			if (!this->InitComponent(component))
			{
				SayNoDebugFatal("Init " << component->GetTypeName() << " failure");
				return false;
			}
		}

		SayNoAssertRetFalse_F(this->StartNetThread());
		this->mCoroutienComponent->StartCoroutine(&App::StartComponent, this);
		return true;
	}

	bool App::InitComponent(Component * component)
	{
		if (!component->IsActive() || !component->Awake())
		{
			SayNoDebugError("init " << component->GetTypeName() << " fail");
			return false;
		}

		if (IFrameUpdate *manager1 = dynamic_cast<IFrameUpdate *>(component))
		{
			this->mFrameUpdateManagers.push_back(manager1);
		}
		if (ISystemUpdate *manager2 = dynamic_cast<ISystemUpdate *>(component))
		{
			this->mSystemUpdateManagers.push_back(manager2);
		}
		if (ISecondUpdate *manager3 = dynamic_cast<ISecondUpdate *>(component))
		{
			this->mSecondUpdateManagers.push_back(manager3);
		}
		if (INetSystemUpdate *manager4 = dynamic_cast<INetSystemUpdate *>(component))
		{
			this->mNetSystemUpdateManagers.push_back(manager4);
		}
		return true;
	}

	bool App::StartNetThread()
	{
	    auto func = std::bind(&App::NetworkLoop, this);
	    this->mNetWorkThread = new std::thread(func);
        this->mMainThreadId = std::this_thread::get_id();
        this->mNetWorkThreadId = this->mNetWorkThread->get_id();
        this->mNetWorkThread->detach();
        return true;
	}

	void App::StartComponent()
    {
        for (int index = 0; index < this->mSceneComponents.size(); index++)
        {
            Component *component = this->mSceneComponents[index];
            if (component != nullptr)
            {
                float process = index / (float) this->mSceneComponents.size();
                SayNoDebugInfo("[" << process * 100 << "%]"
                                   << " start component " << component->GetTypeName());
                component->Start();
            }
        }

        this->mMainLoopStartTime = TimeHelper::GetMilTimestamp();
        SayNoDebugLog("start all scene component successful ......");
        long long t = TimeHelper::GetMilTimestamp() - this->mStartTime;

        for (Component *component: this->mSceneComponents)
        {
            if (ILoadData *loadComponent = dynamic_cast<ILoadData *>(component))
            {
                loadComponent->OnLodaData();
                SayNoDebugLog("load " << component->GetTypeName() << " data");
            }
        }
        this->mIsInitComplate = true;
        SayNoDebugLog("=====  start " << this->mServerName << " successful [" << t / 1000.0f << "s] ========");

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
        spdlog::drop_all();
#ifdef _WIN32
        return getchar();
#endif
        return -1;
    }

	void App::Hotfix()
	{
		for (Component * component : this->mSceneComponents)
		{
			if (auto hotfix = dynamic_cast<IHotfix*>(component))
			{
				hotfix->OnHotFix();
			}
		}
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
       

        std::chrono::milliseconds time(1);
        const long long LogicUpdateInterval = 1000 / logicFps;
        while (!this->mIsClose)
        {
            std::this_thread::sleep_for(time);
            startTimer = TimeHelper::GetMilTimestamp();
            for (ISystemUpdate *component: this->mSystemUpdateManagers)
            {
                component->OnSystemUpdate();
            }

            if (!this->mIsInitComplate)
            {
                continue;
            }
            if (startTimer - mLastUpdateTime >= LogicUpdateInterval)
            {
                this->mLogicRunCount++;
                for (IFrameUpdate *component: this->mFrameUpdateManagers)
                {
                    component->OnFrameUpdate(this->mDelatime);
                }

                for (ILastFrameUpdate *component: this->mLastFrameUpdateManager)
                {
                    component->OnLastFrameUpdate();
                }
                startTimer = mLastUpdateTime = TimeHelper::GetMilTimestamp();
            }

            if (startTimer - secondTimer >= 1000)
            {
                for (ISecondUpdate *component: this->mSecondUpdateManagers)
                {
                    component->OnSecondUpdate();
                }
                this->UpdateConsoleTitle();
                secondTimer = TimeHelper::GetMilTimestamp();
            }
        }
        return this->Stop();
    }

    void App::NetworkLoop()
    {
	    asio::error_code err;
        std::chrono::milliseconds time(1);
        while (!this->mIsClose)
        {
            mAsioContext->poll(err);
			for (INetSystemUpdate * component : this->mNetSystemUpdateManagers)
			{
				component->OnNetSystemUpdate(*mAsioContext);
			}
			std::this_thread::sleep_for(time);
        }
        SayNoDebugError("net thread logout");
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
        //SayNoDebugInfo("fps = " << this->mLogicFps);
#endif
    }
}// namespace Sentry