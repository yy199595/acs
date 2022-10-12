
#include"App.h"
#include"Timer/ElapsedTimer.h"
#include"File/DirectoryHelper.h"
#include"Service/LuaService.h"
#include"Component/ProtoComponent.h"
#include"Component/RedisChannelComponent.h"

using namespace Sentry;
using namespace std::chrono;

namespace Sentry
{
	std::shared_ptr<App> App::mApp = nullptr;

	App::App(ServerConfig* config)
			: Unit(0),
              mStartTime(Helper::Time::GetNowMilTime()), mConfig(config)
	{
        this->mTickCount = 0;
		this->mLogicRunCount = 0;
		this->mTimerComponent = nullptr;
        this->mThreadId = std::this_thread::get_id();
	}

	bool App::LoadComponent()
	{
		this->mTaskComponent = this->GetOrAddComponent<TaskComponent>();
		this->mLogComponent = this->GetOrAddComponent<LoggerComponent>();
		this->mTimerComponent = this->GetOrAddComponent<TimerComponent>();
		this->mMessageComponent = this->GetOrAddComponent<ProtoComponent>();

        std::vector<std::string> components;
		if (this->mConfig->GetMember("component", components)) //添加组件
		{
			for (const std::string& name: components)
			{
                try
                {
                    Component * component = ComponentFactory::CreateComponent(name);
                    if(component == nullptr || !this->AddComponent(name, component))
                    {
                        CONSOLE_LOG_ERROR("add " << name << " failure");
                        return false;
                    }
                }
                catch (std::exception & e)
                {
                    CONSOLE_LOG_ERROR("Init" + name + "error:" + e.what());
                    return false;
                }
			}
		}
        this->GetComponents(components);
		for (const std::string& name: components)
		{
			Component* component = this->GetComponentByName(name);
			if (!this->InitComponent(component))
			{
				CONSOLE_LOG_ERROR("Init " << name << " failure");
				return false;
			}
		}
        this->mTaskComponent->Start(&App::StartAllComponent, this);
        return true;
	}

	bool App::InitComponent(Component* component)
	{
		if (!component->LateAwake())
		{
			LOG_ERROR(component->GetName() << " late awake ");
			return false;
		}

        Service * serviceComponent = component->Cast<Service>();
        IFrameUpdate* manager1 = component->Cast<IFrameUpdate>();
		ISystemUpdate* manager2 = component->Cast<ISystemUpdate>();
		ISecondUpdate* manager3 = component->Cast<ISecondUpdate>();
		ILastFrameUpdate* manager4 = component->Cast<ILastFrameUpdate>();
        RedisChannelComponent * eveComponent = component->Cast<RedisChannelComponent>();

        TryInvoke(eveComponent, eveComponent->StartRegisterEvent());
        TryInvoke(manager1, this->mFrameUpdateManagers.emplace_back(manager1));
		TryInvoke(manager2, this->mSystemUpdateManagers.emplace_back(manager2));
		TryInvoke(manager3, this->mSecondUpdateManagers.emplace_back(manager3));
		TryInvoke(manager4, this->mLastFrameUpdateManager.emplace_back(manager4));
		TryInvoke(serviceComponent, this->mSeviceMap.emplace(component->GetName(), serviceComponent));
		return true;
	}

	int App::Run()
	{
		App::mApp = this->Cast<App>();
		IF_THROW_ERROR(this->mConfig->LoadConfig());
		this->mServerName = this->mConfig->GetNodeName();

		if(!this->LoadComponent())
		{
			this->GetLogger()->SaveAllLog();
			return -1;
		}
		this->mFps = 15;
		mConfig->GetMember("fps", this->mFps);
		this->mLogicUpdateInterval = 1000 / this->mFps;
		this->mStartTime = Helper::Time::GetNowMilTime();
		this->mSecondTimer = Helper::Time::GetNowMilTime();
		this->mLastUpdateTime = Helper::Time::GetNowMilTime();

        std::chrono::milliseconds time(1);
        this->mMainThread = new Asio::Context ();
        Asio::ContextWork * work = new Asio::ContextWork (*mMainThread);
        while(!this->mMainThread->stopped())
        {
            Asio::Code err;
            this->mMainThread->poll(err);
            if(err)
            {
                CONSOLE_LOG_ERROR(err.message());
            }
            this->LogicMainLoop();
            std::this_thread::sleep_for(time);
        }
        delete work;
		return 0;
	}

	void App::Stop()
	{
		if (this->mTaskComponent != nullptr)
		{
			std::shared_ptr<ElapsedTimer> timer(new ElapsedTimer());
			this->mTaskComponent->Start([this, timer]()
			{
				this->OnDestory();
                this->mMainThread->stop();
				LOG_WARN("close server successful " << timer->GetMs() << " ms");
			});
		}
	}

	void App::LogicMainLoop()
	{
		this->mStartTimer = Helper::Time::GetNowMilTime();
		for (ISystemUpdate* component: this->mSystemUpdateManagers)
		{
			component->OnSystemUpdate();
		}

		if (this->mStartTimer - mLastUpdateTime >= this->mLogicUpdateInterval)
		{
			this->mLogicRunCount++;
			for (IFrameUpdate* component: this->mFrameUpdateManagers)
			{
				component->OnFrameUpdate(this->mDeltaTime);
			}

			if (this->mStartTimer - this->mSecondTimer >= 1000)
			{
                this->mTickCount++;
				this->UpdateConsoleTitle();
				for (ISecondUpdate* component: this->mSecondUpdateManagers)
				{
					component->OnSecondUpdate(this->mTickCount);
				}
				this->mSecondTimer = Helper::Time::GetNowMilTime();
			}

			for (ILastFrameUpdate* component: this->mLastFrameUpdateManager)
			{
				component->OnLastFrameUpdate();
			}
			this->mLastUpdateTime = Helper::Time::GetNowMilTime();
		}
	}

	void App::StartAllComponent()
    {
        std::vector<std::string> components;
        this->GetComponents(components);
        for (const std::string &name: components)
        {
            ElapsedTimer timer;
            Component *component = this->GetComponentByName(name);
            long long timeId = this->mTimerComponent->DelayCall(10.0f, [component]() {
                LOG_FATAL(component->GetName() << " start time out");
            });
            if (component->Cast<IStart>() != nullptr)
            {
                if (!component->Cast<IStart>()->Start())
                {
					LOG_FATAL("start " << component->GetName() << " failure");
                    this->Stop();
					return;
                }
            }
            if(timer.GetSecond() > 0)
            {
                LOG_DEBUG("start " << name << " successful use time = [" << timer.GetSecond() << "s]")
            }
            this->mTimerComponent->CancelTimer(timeId);
        }

        CONSOLE_LOG_DEBUG("start all component complete");
        for (const std::string &name: components)
        {
            IComplete *complete = this->GetComponent<IComplete>(name);
            if(complete != nullptr)
            {
                complete->OnComplete();
            }
        }
        this->WaitAllServiceStart();
    }

	void App::WaitAllServiceStart()
	{
        std::vector<std::string> components;
        this->mConfig->GetServices(components);
		for (const std::string name: components)
		{
			int count = 0;
			IServiceBase* serviceBase = this->GetComponent<IServiceBase>(name);
			while (serviceBase != nullptr && !serviceBase->IsStartComplete())
			{
				this->mTaskComponent->Sleep(2000);
				LOG_WARN("wait " << name << " start count = " << ++count);
			}
		}
        std::vector<Component *> componentList;
        this->GetComponents(componentList);
        for (Component * component: componentList)
        {
            IComplete* complete = component->Cast<IComplete>();
            if (complete != nullptr)
            {
                complete->OnAllServiceStart();
            }
        }
		long long t = Helper::Time::GetNowMilTime() - this->mStartTime;
		LOG_INFO("===== start " << this->mServerName << " successful [" << t / 1000.0f << "]s ===========");
	}

	void App::UpdateConsoleTitle()
	{
		long long nowTime = Helper::Time::GetNowMilTime();
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

	Service* App::GetService(const std::string& name)
	{
		auto iter = this->mSeviceMap.find(name);
		return iter != this->mSeviceMap.end() ? iter->second : nullptr;
	}

	bool App::GetServices(std::vector<Service*>& services)
	{
		services.clear();
		auto iter = this->mSeviceMap.begin();
		services.reserve(this->mSeviceMap.size());
		for(; iter != this->mSeviceMap.end(); iter++)
		{
			services.emplace_back(iter->second);
		}
		return !services.empty();
	}
}