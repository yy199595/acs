
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
		this->mIsComplete = false;
		this->mTimerComponent = nullptr;
		this->mMainThreadId = std::this_thread::get_id();
	}

	bool App::LoadComponent()
	{
		std::string path;
		if(!this->mConfig->GetMember("path", "rpc", path))
		{
			throw std::logic_error("not find rpc config");
		}
		if(!this->mRpcConfig.LoadConfig(path))
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
		IF_THROW_ERROR(this->mConfig->GetMember("service", components));
		IF_THROW_ERROR(this->mConfig->GetMember("component", components));

		for(const std::string & name : components)
		{
			if (!this->AddComponentByName(name))
			{
				throw std::logic_error("add " + name + " failure");
				return false;
			}
		}
		components.clear();
		this->mRpcConfig.GetServices(components);
		for(const std::string & name : components)
		{
			Component * component = this->GetComponent<RpcServiceNode>(name);
			if(component == nullptr)
			{
				this->AddComponent(name, new RpcServiceNode());
				LOG_INFO("add remote service : [{0}]", name);
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

	void App::OnAddComponent(Component* component)
	{
		if(component != nullptr)
		{
			this->mNewComponents.push(component);
		}
	}

	void App::OnDelComponent(Component* component)
	{

	}

	bool App::InitComponent(Component* component)
	{
		if (!component->LateAwake())
		{
			LOG_ERROR("{0} late awake ", component->GetName());
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

	void App::StartComponent(Component* component)
	{
		ElapsedTimer elapsedTimer;
		IStart* startComponent = component->Cast<IStart>();
		ILoadData* loadComponent = component->Cast<ILoadData>();
		if (startComponent != nullptr) startComponent->OnStart();
		if (loadComponent != nullptr) loadComponent->OnLoadData();

		LocalServerRpc * localServerRpc = component->Cast<LocalServerRpc>();
		if(localServerRpc != nullptr && this->mIsComplete)
		{
			std::vector<std::string> components;
			this->GetComponents(components);
			for(const std::string & name : components)
			{
				IServiceChange * serviceChange = this->GetComponent<IServiceChange>(name);
				if(serviceChange != nullptr)
				{
					serviceChange->OnAddService(localServerRpc);
				}
			}
		}

		LOG_DEBUG("start {0} user time = {1} ms", component->GetName(), elapsedTimer.GetMs());
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
		this->mTaskComponent->Start(&App::WaitAllServiceStart, this);
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
		this->mStartTimer = Helper::Time::GetNowMilTime();
		for (ISystemUpdate* component : this->mSystemUpdateManagers)
		{
			component->OnSystemUpdate();
		}

		if (this->mStartTimer - mLastUpdateTime >= this->mLogicUpdateInterval)
		{
			this->mLogicRunCount++;
			this->StartNewComponent();
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
				this->mSecondTimer = Helper::Time::GetNowMilTime();
			}

			for (ILastFrameUpdate* component : this->mLastFrameUpdateManager)
			{
				component->OnLastFrameUpdate();
			}
			this->mStartTimer = mLastUpdateTime = Helper::Time::GetNowMilTime();
		}
	}

	bool App::StartNewComponent()
	{
		if(this->mNewComponents.empty())
		{
			return false;
		}
		std::vector<std::string> components;
		while(!this->mNewComponents.empty())
		{
			Component * component = this->mNewComponents.front();
			LOG_CHECK_RET_FALSE(this->InitComponent(component));

			this->mNewComponents.pop();
			components.emplace_back(component->GetName());
		}
		this->mTaskComponent->Start([components, this]()
		{
			for(const std::string & name : components)
			{
				Component* component = this->GetComponent<Component>(name);
				if(component != nullptr)
				{
					this->StartComponent(component);
				}
			}
			for(const std::string & name : components)
			{
				IComplete* complete = this->GetComponent<IComplete>(name);
				if(complete != nullptr)
				{
					complete->OnComplete();
				}
			}
			this->mIsComplete = true;
		});
		return true;
	}

	void App::WaitAllServiceStart()
	{
		std::vector<std::string> components;
		this->GetComponents(components);
		for (const std::string& name : components)
		{
			int count = 0;
			RpcServiceNode* rpcServiceNode = this->GetComponent<RpcServiceNode>(name);
			while (rpcServiceNode != nullptr && !rpcServiceNode->IsStartComplete())
			{
				this->mTaskComponent->Sleep(1000);
				LOG_WARN("wait {0} start [count = {1}]", name, ++count);
				rpcServiceNode = this->GetComponent<RpcServiceNode>(name);
			}
		}
		components.clear();
		this->GetComponents(components);
		for (const std::string& name : components)
		{
			IComplete* complete = this->GetComponent<IComplete>(name);
			if (complete != nullptr)
			{
				complete->OnAllServiceStart();
			}
		}
		long long t = Helper::Time::GetNowMilTime() - this->mStartTime;
		LOG_DEBUG("===== start {0} successful [{1}]s ===========", this->mServerName, t / 1000.0f);
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
		RpcServiceNode* rpcServiceNode = this->GetComponent<RpcServiceNode>(name);
		if (rpcServiceNode->Cast<LocalServerRpc>() == nullptr)
		{
			Component* newComponent = rpcServiceNode->Copy(name);
			if (newComponent != nullptr)
			{
				this->RemoveComponent(name);
				return this->AddComponent(name, newComponent);
			}
		}
		return false;
	}
}