
#include "App.h"

#include <Util/FileHelper.h>
#include <Scene/ActionComponent.h>
#include <Scene/ProtocolComponent.h>
#include <Scene/TcpNetSessionComponent.h>
#include <Service/ServiceNodeComponent.h>
#include <Service/ServiceMgrComponent.h>
#include <Scene/TaskPoolComponent.h>
using namespace Sentry;
using namespace std::chrono;

namespace Sentry
{
    App *App::mApp = nullptr;

	App::App(const std::string srvName, const std::string cfgDir)
		:GameObject(0), mStartTime(TimeHelper::GetMilTimestamp()),
		mConfig(cfgDir + srvName + ".json")
    {
        mApp = this;
        this->mDelatime = 0;
        this->mIsClose = false;
        this->mServerName = srvName;
        this->mLogicRunCount = 0;
        this->mSystemRunCount = 0;
        this->mIsInitComplate = false;
        this->mConfigDir = cfgDir;

        this->mTcpContext = new AsioContext(1);
        this->mHttpContext = new AsioContext(1);
        this->mTcpWork = new AsioWork(*this->mTcpContext);
        this->mHttpWork = new AsioWork(*this->mHttpContext);
        LogHelper::Init("./Logs", this->mServerName);
        this->mNextRefreshTime = TimeHelper::GetTomorrowZeroTime() * 1000;

    }

    void App::OnZeroRefresh()
    {
        spdlog::drop_all();
        LogHelper::Init("./Logs", this->mServerName);
        for (Component * component : this->mSceneComponents)

        {
            if (auto zeroComponent = dynamic_cast<IZeroRefresh*>(component))
            {
                zeroComponent->OnZeroRefresh();
            }
        }
    }

    bool App::LoadComponent()
    {
		this->AddComponent<TimerComponent>();
		this->AddComponent<ActionComponent>();
		this->AddComponent<ProtocolComponent>();
		this->AddComponent<CoroutineComponent>();
		this->AddComponent<TcpNetSessionComponent>();

		this->AddComponent<ServiceNodeComponent>();
		this->AddComponent<ServiceMgrComponent>();

		this->mTimerComponent = this->GetComponent<TimerComponent>();
		this->mCoroutienComponent = this->GetComponent<CoroutineComponent>();



		std::vector<std::string> services;
        std::vector<std::string> components;
        if (!mConfig.GetValue("Scene", components))
        {
            SayNoDebugError("not find field : Managers");
            return false;
        }

		if (!mConfig.GetValue("Service", services))
		{
			SayNoDebugError("not find field : Service");
			return false;
		}

        for(const std::string & name : components)
        {
            if (!this->AddComponent(name))
            {
                SayNoDebugFatal("add " << name << " to service failure");
                return false;
            }
        }

        for(const std::string & name : services)
        {
            if (!this->AddComponent(name))
            {
                SayNoDebugFatal("add " << name << " to scene failure");
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

        for (Component *component: mSceneComponents)
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
		if (ITcpContextUpdate *manager4 = dynamic_cast<ITcpContextUpdate *>(component))
		{
			this->mTcpUpdateComponent.push_back(manager4);
		}

        if (IHttpContextUpdate *manager5 = dynamic_cast<IHttpContextUpdate *>(component))
        {
            this->mHttpUpdateComponent.push_back(manager5);
        }
		return true;
	}

	bool App::StartNetThread()
    {
        TaskPoolComponent *taskComponent = this->GetComponent<TaskPoolComponent>();
        if (taskComponent == nullptr)
        {
            return false;
        }
        auto tcpThread = taskComponent->NewNetworkThread("Tcp", NewMethodProxy(&App::TcpThreadLoop, this));
        auto httpThread = taskComponent->NewNetworkThread("Http", NewMethodProxy(&App::HttpThreadLoop, this));

        SayNoAssertRetFalse_F(tcpThread != nullptr && httpThread != nullptr);

        this->mTcpThreadId = tcpThread->GetThreadId();
        this->mHttpThreadId = httpThread->GetThreadId();
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
		this->mIsInitComplate = true;
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
		this->OnDestory();
        this->mIsClose = true;		
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
                if (secondTimer - this->mNextRefreshTime >= 0)
                {
                    this->OnZeroRefresh();
                    this->mNextRefreshTime = TimeHelper::GetTomorrowZeroTime() * 1000;
                }
            }
        }
        return this->Stop();
    }

    void App::TcpThreadLoop()
    {
        this->mTcpContext->poll();
        for (ITcpContextUpdate *component: this->mTcpUpdateComponent)
        {
            component->OnTcpContextUpdate(*mTcpContext);
        }
    }

    void App::HttpThreadLoop()
    {
        this->mHttpContext->poll();
        for (IHttpContextUpdate *component: this->mHttpUpdateComponent)
        {
            component->OnHttpContextUpdate(*mHttpContext);
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
        //SayNoDebugInfo("fps = " << this->mLogicFps);
#endif
    }
}// namespace Sentry