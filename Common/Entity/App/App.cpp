
#include"App.h"
#include"App/System/System.h"
#include"Timer/ElapsedTimer.h"
#include"File/DirectoryHelper.h"
#include"Service/LuaRpcService.h"
#include"Component/ProtoComponent.h"
#include"Component/LocationComponent.h"
#include"Component/TextConfigComponent.h"
#include"Component/NetThreadComponent.h"
#include"Component/ClusterComponent.h"
#include"Component/RedisChannelComponent.h"
using namespace Sentry;
using namespace std::chrono;

namespace Sentry
{

	App::App() : Unit(0),
        mStartTime(Helper::Time::GetNowMilTime())
	{
        this->mFps = 15;
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

        LOG_CHECK_RET_FALSE(this->AddComponent<TextConfigComponent>());
        LOG_CHECK_RET_FALSE(this->AddComponent<LocationComponent>());
        LOG_CHECK_RET_FALSE(this->AddComponent<NetThreadComponent>());
        LOG_CHECK_RET_FALSE(this->AddComponent<ClusterComponent>());

        std::vector<Component *> components;
        if(this->GetComponents(components) > 0)
        {
            for (Component *component: components)
            {
                if (!this->InitComponent(component))
                {
                    CONSOLE_LOG_ERROR("Init " << component->GetName() << " failure");
                    return false;
                }
            }
        }
        ServerConfig::Inst()->GetMember("fps", this->mFps);
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

        RpcService * serviceComponent = component->Cast<RpcService>();
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

	int App::Run(int argc, char ** argv)
	{
        if(argc != 3)
        {
            return -1;
        }
        System::Init(argv);
		if(!this->LoadComponent())
		{
			this->GetLogger()->SaveAllLog();
			return -1;
		}
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
        this->mMainThread->stop();
        LOG_WARN("close server successful ");
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
        std::vector<std::string > components;
        this->GetComponents(components);
        for (const std::string &name: components)
        {
            ElapsedTimer timer;
            IStart * component = this->GetComponent<IStart>(name);
            if(component != nullptr)
            {
                long long timeId = this->mTimerComponent->DelayCall(10 * 1000, [name]()
                {
                    LOG_FATAL(name << " start time out");
                });
                if(!component->Start())
                {
                    LOG_ERROR("start [" << name << "] failure");
                    this->Stop();
                    return;
                }
                this->mTimerComponent->CancelTimer(timeId);
                LOG_DEBUG("start " << name << " successful use time = [" << timer.GetSecond() << "s]");
            }
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
        std::vector<IServiceBase *> components;
        this->GetComponents<IServiceBase>(components);
		for (IServiceBase * component: components)
        {
            int count = 0;
            while (!component->IsStartComplete())
            {
                this->mTaskComponent->Sleep(2000);
                LOG_WARN("wait " << dynamic_cast<Component*>(component)->GetName() << " start count = " << ++count);
            }
        }
        std::vector<IComplete *> completeComponents;
        this->GetComponents<IComplete>(completeComponents);
        for (IComplete* complete: completeComponents)
        {
            complete->OnAllServiceStart();
        }
		long long t = Helper::Time::GetNowMilTime() - this->mStartTime;
		LOG_INFO("===== start " << System::GetName() << " successful [" << t / 1000.0f << "]s ===========");
	}

	void App::UpdateConsoleTitle()
	{
		long long nowTime = Helper::Time::GetNowMilTime();
		float seconds = (nowTime - this->mSecondTimer) / 1000.0f;
		this->mLogicFps = (float)this->mLogicRunCount / seconds;
#ifdef _WIN32
		char buffer[100] = {0};
		const std::string& name = System::GetName();
		sprintf_s(buffer, "%s fps:%f", name.c_str(), this->mLogicFps);
		SetConsoleTitle(buffer);
#else
		//LOG_INFO("fps = " << this->mLogicFps);
#endif
		this->mLogicRunCount = 0;
	}

	RpcService* App::GetService(const std::string& name)
	{
		auto iter = this->mSeviceMap.find(name);
		return iter != this->mSeviceMap.end() ? iter->second : nullptr;
	}

	bool App::GetServices(std::vector<RpcService*>& services)
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