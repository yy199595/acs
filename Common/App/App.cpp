
#include"App.h"
#include"Other/ElapsedTimer.h"
#include"Util/DirectoryHelper.h"
#include"Component/Service/LuaRpcService.h"
#include"Component/Scene/ServiceMgrComponent.h"
#include"Component/Lua/LuaScriptComponent.h"
#ifdef __DEBUG__
#include"Component/Telnet/ConsoleComponent.h"
#endif
using namespace Sentry;
using namespace std::chrono;

namespace Sentry
{
	std::shared_ptr<App> App::mApp = nullptr;
	App::App(ServerConfig* config)
		: Entity(0),
		  mConfig(config), mStartTime(Helper::Time::GetMilTimestamp()),
		  mTaskScheduler(NewMethodProxy(&App::LogicMainLoop, this))
	{
		this->mLogicRunCount = 0;
		this->mTimerComponent = nullptr;
		this->mMainThreadId = std::this_thread::get_id();
	}

	bool App::LoadComponent()
	{
		this->AddComponent<LoggerComponent>();
		this->mLogComponent = this->GetComponent<LoggerComponent>();
		LOG_CHECK_RET_FALSE(this->AddComponent<TaskComponent>());
		LOG_CHECK_RET_FALSE(this->AddComponent<TimerComponent>());
		this->mTaskComponent = this->GetComponent<TaskComponent>();
		this->mTimerComponent = this->GetComponent<TimerComponent>();

		std::vector<std::string> components;
		IF_THROW_ERROR(this->mConfig->GetMember("component", components));
		IF_THROW_ERROR(this->mConfig->GetMember("service", components));

		for (const std::string& name : components)
		{
			if (!this->AddComponentByName(name))
			{
				throw std::logic_error("add " + name + " failure");
				return false;
			}
			//LOG_DEBUG("add new component : " << name);
		}
		return true;
	}

	bool App::AddComponentByName(const std::string& name)
	{
		Component* component = ComponentFactory::CreateComponent(name);
		if (component != nullptr)
		{
			return this->AddComponent(name, component);
		}
		LuaRpcService* luaRpcService = new LuaRpcService();
		return this->AddComponent(name, luaRpcService);
	}

	bool App::InitComponent()
	{
		// 初始化scene组件
		std::vector<Component*> components;
		this->GetComponents(components);
		for (Component* component : components)
		{
			if (!this->InitComponent(component))
			{
				LOG_FATAL("Init {0} failure", component->GetName());
				return false;
			}
			LOG_DEBUG("Init {0} successful", component->GetName());
		}
		this->mTaskComponent->Start([this]()
		{
			for (Component* component : this->mSceneComponents)
			{
				this->StartComponent(component);
			}
			for (Component* component : this->mSceneComponents)
			{
				IComplete * complete = component->Cast<IComplete>();
				if(complete != nullptr)
				{
					complete->OnComplete();
				}
			}
			long long t = Helper::Time::GetMilTimestamp() - this->mStartTime;
			LOG_DEBUG("===== start {0} successful [{1}]s ===========", this->mServerName, t / 1000.0f);
		});
		return true;
	}

	bool App::InitComponent(Component* component)
	{
		if (!component->LateAwake())
		{
			return false;
		}
		IFrameUpdate * manager1 = component->Cast<IFrameUpdate>();
		ISystemUpdate * manager2 = component->Cast<ISystemUpdate>();
		ISecondUpdate * manager3 = component->Cast<ISecondUpdate>();
		ILastFrameUpdate * manager4 = component->Cast<ILastFrameUpdate>();
		TryInvoke(manager1, this->mFrameUpdateManagers.emplace_back(manager1));
		TryInvoke(manager2, this->mSystemUpdateManagers.emplace_back(manager2));
		TryInvoke(manager3, this->mSecondUpdateManagers.emplace_back(manager3));
		TryInvoke(manager4, this->mLastFrameUpdateManager.emplace_back(manager4));
		this->mSceneComponents.emplace_back(component);
		return true;
	}

	void App::StartComponent(Component* component)
	{
		ElapsedTimer elapsedTimer;
		IStart * startComponent = component->Cast<IStart>();
		ILoadData * loadComponent = component->Cast<ILoadData>();
		if (startComponent != nullptr) startComponent->OnStart();
		if (loadComponent != nullptr) loadComponent->OnLoadData();
		LOG_DEBUG("start {0} user time = {1} ms", component->GetName(), elapsedTimer.GetMs());
	}

	int App::Run()
	{
		App::mApp = this->Cast<App>();
		IF_THROW_ERROR(this->mConfig->LoadConfig());
		this->mServerName = this->mConfig->GetNodeName();

		IF_THROW_ERROR(this->LoadComponent());
		IF_THROW_ERROR(this->InitComponent());

		this->mFps = 15;
		mConfig->GetMember("fps", this->mFps);
		this->mLogicUpdateInterval = 1000 / this->mFps;
		this->mStartTime = Helper::Time::GetMilTimestamp();
		this->mSecondTimer = Helper::Time::GetMilTimestamp();
		this->mLastUpdateTime = Helper::Time::GetMilTimestamp();

		return this->mTaskScheduler.Start();
	}

	void App::Stop()
	{
		if (this->mTaskComponent != nullptr)
		{
			std::shared_ptr<ElapsedTimer> timer(new ElapsedTimer());
			this->mTaskComponent->Start([this, timer]()
			{
				this->OnDestory();
				this->mTaskScheduler.Stop();
				LOG_WARN("close server successful [{0}]ms", timer->GetMs());
			});
		}
	}

	void App::LogicMainLoop()
	{
		this->mStartTimer = Helper::Time::GetMilTimestamp();
		for (ISystemUpdate* component : this->mSystemUpdateManagers)
		{
			component->OnSystemUpdate();
		}

		if (this->mStartTimer - mLastUpdateTime >= this->mLogicUpdateInterval)
		{
			this->mLogicRunCount++;
			for (IFrameUpdate* component : this->mFrameUpdateManagers)
			{
				component->OnFrameUpdate(this->mDeltaTime);
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

			for (ILastFrameUpdate* component : this->mLastFrameUpdateManager)
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