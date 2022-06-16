
#include"App.h"
#include"Util/FileHelper.h"
#include"Other/ElapsedTimer.h"
#include"Util/DirectoryHelper.h"
#include"Component/Scene/MessageComponent.h"
#include"Component/RpcService/LocalLuaService.h"
#include"Component/RpcService/ServiceAgentComponent.h"
using namespace Sentry;
using namespace std::chrono;

namespace Sentry
{
	std::shared_ptr<App> App::mApp = nullptr;

	App::App(ServerConfig* config)
			: Entity(0),
			  mStartTime(Helper::Time::GetNowMilTime()), mConfig(config)
	{
        this->mTickCount = 0;
		this->mLogicRunCount = 0;
		this->mTimerComponent = nullptr;
		StaticMethod * staticMethod = NewMethodProxy(&App::LogicMainLoop, this);
		this->mTaskScheduler = std::make_shared<MainTaskScheduler>(staticMethod);
	}

	bool App::LoadComponent()
	{
		std::string path;
		if (!this->mConfig->GetPath("service", path))
		{
			CONSOLE_LOG_ERROR("not find serice config");
			return false;
		}

		rapidjson::Document jsonDocument;
		if(!Helper::File::ReadJsonFile(path, jsonDocument))
		{
			CONSOLE_LOG_ERROR("load service [" << path << "] error");
			return false;
		}

		this->mTaskComponent = this->GetOrAddComponent<TaskComponent>();
		this->mLogComponent = this->GetOrAddComponent<LoggerComponent>();
		this->mTimerComponent = this->GetOrAddComponent<TimerComponent>();
		this->mMessageComponent = this->GetOrAddComponent<MessageComponent>();

		std::vector<std::string> components;
		if (this->mConfig->GetMember("component", components)) //添加组件
		{
			for (const std::string& name: components)
			{
				Component * component = ComponentFactory::CreateComponent(name);
				if(component == nullptr || !this->AddComponent(name, component))
				{
					CONSOLE_LOG_ERROR("add " << name << " failure");
					return false;
				}
			}
		}
		auto iter = jsonDocument.MemberBegin();
		for(;iter != jsonDocument.MemberEnd(); iter++)
		{
			const rapidjson::Value & josnValue = iter->value;
            const std::string name(iter->name.GetString());
            std::string type(josnValue["Type"].GetString());
            Component* component = ComponentFactory::CreateComponent(name);
            if(component == nullptr)
            {
                component = ComponentFactory::CreateComponent(type);
                if(component == nullptr && type == "rpc")
                {
                    component = new ServiceAgentComponent();
                }
            }
			if(component == nullptr || !this->AddComponent(name, component))
			{
				CONSOLE_LOG_ERROR("add " << name << " failure");
				return false;
			}
			IServiceBase * serviceBase =component->Cast<IServiceBase>();
			if(serviceBase == nullptr || !serviceBase->LoadConfig(iter->value))
			{
				CONSOLE_LOG_ERROR("load " << name << " config error");
				return false;
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
		return true;
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
		ServiceComponent * serviceComponent = component->Cast<ServiceComponent>();
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
		if(!this->StartNewComponent())
		{
			this->GetLogger()->SaveAllLog();
			return -2;
		}
		this->mFps = 15;
		mConfig->GetMember("fps", this->mFps);
		this->mLogicUpdateInterval = 1000 / this->mFps;
		this->mStartTime = Helper::Time::GetNowMilTime();
		this->mSecondTimer = Helper::Time::GetNowMilTime();
		this->mLastUpdateTime = Helper::Time::GetNowMilTime();
		return this->mTaskScheduler->Start();
	}

	void App::Stop()
	{
		if (this->mTaskComponent != nullptr)
		{
			std::shared_ptr<ElapsedTimer> timer(new ElapsedTimer());
			this->mTaskComponent->Start([this, timer]()
			{
				this->OnDestory();
				this->mTaskScheduler->Stop();
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
				IServiceBase* localServerRpc = component->Cast<IServiceBase>();
				if (localServerRpc != nullptr && !localServerRpc->IsStartService())
				{
					continue;
				}
				if(startComponent != nullptr)
				{
					ElapsedTimer timer;
					long long timeId = this->mTimerComponent->DelayCall(5.0f, [component]()
					{
						LOG_FATAL(component->GetName() << " start time out");
					});
					if(!startComponent->OnStart())
					{
						throw std::logic_error(fmt::format(
							"start {0} failure", component->GetName()));
					}
					this->mTimerComponent->CancelTimer(timeId);
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
				IServiceBase* localServerRpc = component->Cast<IServiceBase>();
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
	}

	bool App::StartNewComponent()
	{
		std::vector<std::string> components;
		this->mConfig->GetMember("service.rpc", components);
        this->mConfig->GetMember("service.http", components);
        for (const std::string& name: components)
		{
			IServiceBase* localServerRpc = this->GetComponent<IServiceBase>(name);
			if (localServerRpc != nullptr && !localServerRpc->StartService())
			{
				LOG_ERROR(name << " load failure");
				return false;
			}
		}
		this->mTaskComponent->Start(&App::StartAllComponent, this);
		this->mTaskComponent->Start(&App::WaitAllServiceStart, this);
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
		LOG_DEBUG("===== start " << this->mServerName << " successful [" << t / 1000.0f << "]s ===========");
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
			LOG_CHECK_RET_FALSE(serviceBase->StartService());

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
		std::vector<Component *> components;
		this->GetComponents(components);
		for(Component * component1 : components)
		{
			IServiceChange * serviceChange = component1->Cast<IServiceChange>();
			if(serviceChange != nullptr)
			{
				serviceChange->OnAddService(component);
			}
		}
	}

	ServiceComponent* App::GetService(const std::string& name)
	{
		auto iter = this->mSeviceMap.find(name);
		return iter != this->mSeviceMap.end() ? iter->second : nullptr;
	}

	bool App::GetServices(std::vector<ServiceComponent*>& services)
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