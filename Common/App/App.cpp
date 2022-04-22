
#include"App.h"
#include"Other/ElapsedTimer.h"
#include"Util/DirectoryHelper.h"
#include"Component/RpcService/LuaRpcService.h"
using namespace Sentry;
using namespace std::chrono;

namespace Sentry
{
	std::shared_ptr<App> App::mApp = nullptr;

	App::App(ServerConfig* config)
			: Entity(0),
			  mConfig(config), mStartTime(Helper::Time::GetNowMilTime()),
			  mTaskScheduler(NewMethodProxy(&App::LogicMainLoop, this))
	{
		this->mLogicRunCount = 0;
		this->mTimerComponent = nullptr;
		this->mMainThreadId = std::this_thread::get_id();
	}

	bool App::LoadComponent()
	{
		std::string path;
		if (!this->mConfig->GetMember("path", "rpc", path))
		{
			throw std::logic_error("not find rpc config");
		}
		if (!this->mRpcConfig.LoadConfig(path))
		{
			throw std::logic_error("load rpc config error");
		}

		this->AddComponent<LoggerComponent>();
		this->mLogComponent = this->GetComponent<LoggerComponent>();
		LOG_CHECK_RET_FALSE(this->AddComponent<TaskComponent>());
		LOG_CHECK_RET_FALSE(this->AddComponent<TimerComponent>());
		this->mTaskComponent = this->GetComponent<TaskComponent>();
		this->mTimerComponent = this->GetComponent<TimerComponent>();

		std::vector<std::string> components;
		if (this->mConfig->GetMember("component", components)) //添加组件
		{
			for (const std::string& name: components)
			{
				if (this->GetComponentByName(name) != nullptr)
				{
					LOG_ERROR("add " << name << " failure");
					return false;
				}
				if (!this->AddComponentByName(name))
				{
					LOG_ERROR("add" << name << " failure");
					return false;
				}
			}
		}
		components.clear();
		if (this->mConfig->GetMember("service", components)) //添加所有服务
		{
			for (const std::string& name: components)
			{
				if (this->GetComponentByName(name) != nullptr)
				{
					LOG_ERROR("add " << name << " failure");
					return false;
				}
				if (!this->AddComponentByName(name))
				{
					LuaRpcService* luaRpcService = new LuaRpcService();
					return this->AddComponent(name, luaRpcService);
				}
			}
		}
		this->GetComponents(components);
		for (const std::string& name: components)
		{
			Component* component = this->GetComponentByName(name);
			if (!this->InitComponent(component))
			{
				return false;
			}
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

	bool App::InitComponent(Component* component)
	{
		if (!component->LateAwake())
		{
			LOG_ERROR(component->GetName() << " late awake ");
			return false;
		}

		IFrameUpdate* manager1 = component->Cast<IFrameUpdate>();
		ISystemUpdate* manager2 = component->Cast<ISystemUpdate>();
		ISecondUpdate* manager3 = component->Cast<ISecondUpdate>();
		ILastFrameUpdate* manager4 = component->Cast<ILastFrameUpdate>();
		TryInvoke(manager1, this->mFrameUpdateManagers.emplace_back(manager1));
		TryInvoke(manager2, this->mSystemUpdateManagers.emplace_back(manager2));
		TryInvoke(manager3, this->mSecondUpdateManagers.emplace_back(manager3));
		TryInvoke(manager4, this->mLastFrameUpdateManager.emplace_back(manager4));
		return true;
	}

	int App::Run()
	{
		App::mApp = this->Cast<App>();
		IF_THROW_ERROR(this->mConfig->LoadConfig());
		this->mServerName = this->mConfig->GetNodeName();

		IF_THROW_ERROR(this->LoadComponent());
		IF_THROW_ERROR(this->StartNewComponent());

		this->mFps = 15;
		mConfig->GetMember("fps", this->mFps);
		this->mLogicUpdateInterval = 1000 / this->mFps;
		this->mStartTime = Helper::Time::GetNowMilTime();
		this->mSecondTimer = Helper::Time::GetNowMilTime();
		this->mLastUpdateTime = Helper::Time::GetNowMilTime();
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
				this->UpdateConsoleTitle();
				for (ISecondUpdate* component: this->mSecondUpdateManagers)
				{
					component->OnSecondUpdate();
				}
				this->mSecondTimer = Helper::Time::GetNowMilTime();
			}

			for (ILastFrameUpdate* component: this->mLastFrameUpdateManager)
			{
				component->OnLastFrameUpdate();
			}
			this->mStartTimer = mLastUpdateTime = Helper::Time::GetNowMilTime();
		}
	}

	void App::StartAllComponent()
	{
		std::vector<std::string> components;
		this->GetComponents(components);
		for (const std::string& name: components)
		{
			Component* component = this->GetComponentByName(name);
			if (component != nullptr)
			{
				IStart* startComponent = component->Cast<IStart>();
				LocalServerRpc* localServerRpc = component->Cast<LocalServerRpc>();
				if (localServerRpc != nullptr && !localServerRpc->IsStartService())
				{
					continue;
				}
				if(startComponent != nullptr)
				{
					ElapsedTimer timer;
					if(!startComponent->OnStart())
					{
						throw std::logic_error(fmt::format(
							"start {0} failure", component->GetName()));
					}
					LOG_DEBUG("start " << name << " successful use time = [" << timer.GetSecond() << "s]")
				}
			}
		}

		LOG_WARN("start all component complete");
		for (const std::string& name: components)
		{
			Component* component = this->GetComponentByName(name);
			if (component != nullptr)
			{
				LocalServerRpc* localServerRpc = component->Cast<LocalServerRpc>();
				if (localServerRpc == nullptr || localServerRpc->IsStartService())
				{
					IComplete* complete = component->Cast<IComplete>();
					if (complete != nullptr)
					{
						complete->OnComplete();
					}
				}
			}
		}
		this->WaitAllServiceStart();
	}

	bool App::StartNewComponent()
	{
		std::vector<std::string> components;
		this->mConfig->GetMember("start", components);
		for (const std::string& name: components)
		{
			IServiceBase* localServerRpc = this->GetComponent<IServiceBase>(name);
			if (localServerRpc != nullptr && !localServerRpc->LoadService())
			{
				LOG_ERROR(name << " load failure");
				return false;
			}
			LOG_INFO(name << " load successful");
		}
		this->mTaskComponent->Start(&App::StartAllComponent, this);
		//this->mTaskComponent->Start(&App::WaitAllServiceStart, this);
		return true;
	}

	void App::WaitAllServiceStart()
	{
		std::vector<std::string> components;
		this->GetComponents(components);
		for (const std::string& name: components)
		{
			int count = 0;
			IServiceBase* serviceBase = this->GetComponent<IServiceBase>(name);
			while (serviceBase != nullptr && !serviceBase->IsStartComplete())
			{
				this->mTaskComponent->Sleep(2000);
				LOG_WARN("wait " << name << " start count = " << ++count);
			}
		}
		for (const std::string& name: components)
		{
			IComplete* complete = this->GetComponent<IComplete>(name);
			if (complete != nullptr)
			{
				complete->OnAllServiceStart();
			}
		}
		long long t = Helper::Time::GetNowMilTime() - this->mStartTime;
		LOG_DEBUG("===== start server successful [" << t / 1000.0f << "]s ===========");
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

	bool App::StartNewService(const std::string& name)
	{
		ElapsedTimer timer;
		Component* component = this->GetComponentByName(name);
		IServiceBase* serviceBase = component->Cast<IServiceBase>();
		if (serviceBase != nullptr && !serviceBase->IsStartService())
		{
			LOG_CHECK_RET_FALSE(serviceBase->LoadService());

			IStart* start = component->Cast<IStart>();
			IComplete* complete = component->Cast<IComplete>();
			if (start != nullptr && !start->OnStart())
			{
				LOG_FATAL("start " << name << " failure");
				return false;
			}
			if (complete != nullptr)
			{
				complete->OnComplete();
			}
			this->OnAddNewService(component);
			LOG_INFO("start " << name << " user time " << timer.GetMs() << " ms");
			return true;
		}
		return false;
	}
	void App::OnAddNewService(Component* component)
	{
		std::vector<std::string> components;
		this->GetComponents(components);
		for(const std::string & name : components)
		{
			IServiceChange * serviceChange = this->GetComponent<IServiceChange>(name);
			if(serviceChange != nullptr)
			{
				serviceChange->OnAddService(component);
			}
		}
	}
}