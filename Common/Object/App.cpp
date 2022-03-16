
#include"App.h"
#include"Other/ElapsedTimer.h"
#include"Util/DirectoryHelper.h"
#include"Service/LuaRpcService.h"
#include"Scene/ServiceMgrComponent.h"
#include"Scene/LuaScriptComponent.h"
#ifdef __DEBUG__
#include"Telnet/ConsoleComponent.h"
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

	bool App::StartNewService(const std::string& name)
	{
		LOG_CHECK_RET_FALSE(!this->GetComponentByName(name));
		Component* component = ComponentFactory::CreateComponent(name);
		if (dynamic_cast<RpcService*>(component) == nullptr)
		{
			delete component;
			return false;
		}
		if (!this->AddComponent(name, component))
		{
			delete component;
			return false;
		}
		if (!this->InitComponent(component))
		{
			this->RemoveComponent(name);
			return false;
		}
		this->StartComponent(component);
		component->OnComplete();
		return true;
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
		IF_THROW_ERROR(this->mConfig->GetValue("component", components));
		IF_THROW_ERROR(this->mConfig->GetValue("service", components));

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
				LOG_FATAL("Init ", component->GetName(), " failure");
				return false;
			}
		}
		this->mTaskComponent->Start([this]()
		{
			for (Component* component : this->mSceneComponents)
			{
				this->StartComponent(component);
			}
			for (Component* component : this->mSceneComponents)
			{
				component->OnComplete();
			}
			long long t = Helper::Time::GetMilTimestamp() - this->mStartTime;
			LOG_DEBUG("===== start ", this->mServerName, " successful [", t / 1000.0f, "]s =======");
		});
		return true;
	}

	bool App::InitComponent(Component* component)
	{
		if (!component->LateAwake()) return false;
		auto manager1 = dynamic_cast<IFrameUpdate*>(component);
		auto manager2 = dynamic_cast<ISystemUpdate*>(component);
		auto manager3 = dynamic_cast<ISecondUpdate*>(component);
		auto manager4 = dynamic_cast<ILastFrameUpdate*>(component);
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
		auto startComponent = dynamic_cast<IStart*>(component);
		auto loadComponent = dynamic_cast<ILoadData*>(component);
		if (startComponent != nullptr) startComponent->OnStart();
		if (loadComponent != nullptr) loadComponent->OnLoadData();
		LOG_DEBUG("start ", component->GetName(), " use time = ", elapsedTimer.GetMs(), "ms");
	}

	int App::Run()
	{
		App::mApp = this->Cast<App>();
		IF_THROW_ERROR(this->mConfig->LoadConfig());
		this->mServerName = this->mConfig->GetNodeName();

		IF_THROW_ERROR(this->LoadComponent());
		IF_THROW_ERROR(this->InitComponent());

		this->mFps = 15;
		mConfig->GetValue("fps", this->mFps);
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
				LOG_WARN("close server successful [", timer->GetMs(), "ms]");
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