
#include"App.h"
#include"Core/System/System.h"
#include"Timer/Timer/ElapsedTimer.h"
#include"Util/File/DirectoryHelper.h"
#include"Rpc/Service/VirtualRpcService.h"
#include"Proto/Component/ProtoComponent.h"
#include"Rpc/Component/NodeMgrComponent.h"
#include"Server/Component/TextConfigComponent.h"
#include"Server/Component/ThreadComponent.h"
#include"Cluster/Component/LaunchComponent.h"
#include<csignal>
#ifdef __OS_WIN__
#include<Windows.h>
#endif

using namespace Sentry;
using namespace std::chrono;

namespace Sentry
{

	App::App() : Unit(0),
        mStartTime(Helper::Time::NowMilTime()),
				 mThreadId(std::this_thread::get_id())
	{
        this->mLogicFps = 0;
        this->mTickCount = 0;
        this->mIsStartDone = false;
        this->mLogComponent = nullptr;
        this->mTaskComponent = nullptr;
        this->mTimerComponent = nullptr;
        this->mMessageComponent = nullptr;
		this->mStatus = ServerStatus::Init;
	}

	bool App::LoadComponent()
	{
#ifndef __OS_WIN__
        signal(SIGQUIT, App::HandleSignal);
        signal(SIGKILL, App::HandleSignal);
#endif
        signal(SIGTERM, App::HandleSignal);
        signal(SIGINT, App::HandleSignal);
        //SetConsoleCtrlHandler((PHANDLER_ROUTINE)WinHandlerSignal, true);
		this->mTaskComponent = this->GetOrAddComponent<AsyncMgrComponent>();
		this->mLogComponent = this->GetOrAddComponent<LogComponent>();
		this->mTimerComponent = this->GetOrAddComponent<TimerComponent>();
		this->mMessageComponent = this->GetOrAddComponent<ProtoComponent>();

        LOG_CHECK_RET_FALSE(this->AddComponent<TextConfigComponent>());
        //LOG_CHECK_RET_FALSE(this->AddComponent<LocationComponent>());
        LOG_CHECK_RET_FALSE(this->AddComponent<ThreadComponent>());
        LOG_CHECK_RET_FALSE(this->AddComponent<LaunchComponent>());
        std::vector<Component *> components;
        if(this->GetComponents(components) > 0)
        {
            for (Component *component: components)
            {
				RpcService * rpcService = component->Cast<RpcService>();
				if(rpcService != nullptr)
				{
					const std::string & name = component->GetName();
					this->mServiceMap.emplace(name, rpcService);
				}
            }
			for (Component *component: components)
			{
				if(!component->LateAwake())
				{
					LOG_ERROR(component->GetName() << " LateAwake");
					return false;
				}
			}
        }
        this->mTaskComponent->Start(&App::StartAllComponent, this);
        return true;
	}

	int App::Run(int argc, char ** argv)
    {
        if (!System::Init(argc, argv))
        {
            CONSOLE_LOG_FATAL("start failure");
#ifdef __OS_WIN__
            return getchar();
#endif
            return -1;
        }
        std::string name = argc >= 3 ? argv[2] : "Server";
        std::unique_ptr<ServerConfig> serverConfig(new ServerConfig(name));

        if (!serverConfig->LoadConfig(System::ConfigPath()))
        {
            CONSOLE_LOG_FATAL("load server config error");
#ifdef __OS_WIN__
            return getchar();
#endif
            return -2;
        }
        this->mMainContext = std::make_unique<Asio::Context>();
        if (!this->LoadComponent())
        {
            this->mLogComponent->SaveAllLog();
#ifdef __OS_WIN__
            return getchar();
#endif
            return -1;
        }
        Asio::ContextWork work(*this->mMainContext);
        const std::chrono::milliseconds sleepTime(1);
        long long logicStartTime = Helper::Time::NowMilTime();
        long long logicSecondTime = Helper::Time::NowMilTime();
        long long logicLastUpdateTime = Helper::Time::NowMilTime();

        std::vector<IFrameUpdate *> frameUpdateComponents;
        std::vector<ISystemUpdate *> systemUpdateComponents;
        std::vector<ISecondUpdate *> secondUpdateComponents;
        std::vector<ILastFrameUpdate *> lastFrameUpdateComponents;
        this->GetComponents<IFrameUpdate>(frameUpdateComponents);
        this->GetComponents<ISystemUpdate>(systemUpdateComponents);
        this->GetComponents<ISecondUpdate>(secondUpdateComponents);
        this->GetComponents<ILastFrameUpdate>(lastFrameUpdateComponents);


        int fps = 15;
        Asio::Code code;
        float deltaTime = 0;
        long long logicRunCount = 0;
        ServerConfig::Inst()->GetMember("fps", fps);
        long long logicUpdateInterval = 1000 / fps;
        while (!this->mMainContext->stopped())
        {
            this->mMainContext->poll(code);
            for (ISystemUpdate *component: systemUpdateComponents)
            {
                component->OnSystemUpdate();
            }
            if (this->mIsStartDone)
			{
				logicStartTime = Helper::Time::NowMilTime();
				if (logicStartTime - logicLastUpdateTime >= logicUpdateInterval)
				{
					logicRunCount++;
					for (IFrameUpdate* component : frameUpdateComponents)
					{
						component->OnFrameUpdate(deltaTime);
					}

					if (logicStartTime - logicSecondTime >= 1000)
					{
						this->mTickCount++;
#ifdef __OS_WIN__
						this->UpdateConsoleTitle();
#endif
						long long nowTime = Helper::Time::NowMilTime();
						float seconds = (nowTime - logicSecondTime) / 1000.0f;
						this->mLogicFps = (float)logicRunCount / seconds;
						for (ISecondUpdate* component : secondUpdateComponents)
						{
							component->OnSecondUpdate(this->mTickCount);
						}
						logicRunCount = 0;
						logicSecondTime = Helper::Time::NowMilTime();
					}

					for (ILastFrameUpdate* component : lastFrameUpdateComponents)
					{
						component->OnLastFrameUpdate();
					}
					logicLastUpdateTime = Helper::Time::NowMilTime();
				}
			}
            std::this_thread::sleep_for(sleepTime);
        }
#ifdef __OS_WIN__
        return std::getchar();
#else
        return 0;
#endif // __OS_WIN__
    }

	void App::Stop()
    {
		if(this->mStatus == ServerStatus::Closing)
		{
			return;
		}
		this->mStatus = ServerStatus::Closing;
#ifdef __DEBUG__
		CONSOLE_LOG_ERROR("start close " << ServerConfig::Inst()->Name());
#endif
		std::vector<IDestroy *> components;
		this->GetComponents<IDestroy>(components);
		std::reverse(components.begin(), components.end());
		for(IDestroy * destroy : components)
		{
#ifdef __DEBUG__
			Component * component = dynamic_cast<Component*>(destroy);
			CONSOLE_LOG_DEBUG(component->GetName() << ".OnDestroy");
#endif
			destroy->OnDestroy();
		}
		for(int index = 5; index >= 0; index--)
		{
			CONSOLE_LOG_INFO("shutdown " <<
				ServerConfig::Inst()->Name() << " in [" << index << "s] after...");
			this->mTaskComponent->Sleep(1000);
		}

		this->mMainContext->stop();
		this->mLogComponent->SaveAllLog();
		LOG_WARN("close " << ServerConfig::Inst()->Name() << " successful ");
	}

	void App::StartAllComponent()
    {
        std::vector<IStart*> startComponents;
        this->GetComponents(startComponents);
        for(IStart * component : startComponents)
        {
            ElapsedTimer timer;
            const std::string & name = dynamic_cast<Component*>(component)->GetName();
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
        this->mIsStartDone = true; //开始帧循环
        std::vector<IComplete *> completeComponents;
        this->GetComponents<IComplete>(completeComponents);
        for(IComplete * complete : completeComponents)
        {
			Component* component = dynamic_cast<Component*>(complete);
			long long timerId = this->mTimerComponent->DelayCall(1000 * 10, [component]()
			{
				CONSOLE_LOG_FATAL(component->GetName() << " [OnLocalComplete] time out");
			});
			complete->OnLocalComplete();
			this->mTimerComponent->CancelTimer(timerId);
        }
		CONSOLE_LOG_DEBUG("start all component complete");
		this->mStatus = ServerStatus::Ready;
		this->WaitServerStart();
    }

	void App::WaitServerStart() //等待依赖的服务启动完成
	{
		NodeMgrComponent* locationComponent = this->GetComponent<NodeMgrComponent>();
		if (locationComponent != nullptr)
		{
			std::unordered_set<std::string> services;
			std::vector<VirtualRpcService*> allVirtualServices;
			this->GetComponents<VirtualRpcService>(allVirtualServices);
			for (const VirtualRpcService* service: allVirtualServices)
			{
				const std::string& server = service->GetServer();
				if (services.find(server) == services.end())
				{
					services.insert(server);
					locationComponent->WaitServerStart(server);
					CONSOLE_LOG_INFO(server << " start successful ...");
				}
			}
		}
		std::vector<IComplete*> completeComponents;
		this->GetComponents<IComplete>(completeComponents);
		for (IComplete* complete: completeComponents)
		{
			Component* component = dynamic_cast<Component*>(complete);
			long long timerId = this->mTimerComponent->DelayCall(1000 * 10, [component]()
			{
				CONSOLE_LOG_FATAL(component->GetName() << " [OnClusterComplete] time out");
			});
			complete->OnClusterComplete();
			this->mTimerComponent->CancelTimer(timerId);
		}
		this->mStatus = ServerStatus::Running;
		long long t = Helper::Time::NowMilTime() - this->mStartTime;
		LOG_INFO("===== start " << ServerConfig::Inst()->Name() << " successful [" << t / 1000.0f << "]s ===========");
	}
	void App::HandleSignal(int signal)
	{
		AsyncMgrComponent* component =
			App::Inst()->GetTaskComponent();
		component->Start(&App::Stop, App::Inst());
	}
#ifdef __OS_WIN__
	void App::UpdateConsoleTitle()
	{       
        std::string title = ServerConfig::Inst()->Name();
        //HttpWebComponent * httpComponent = this->GetComponent<HttpWebComponent>();
        //MongoDBComponent* mongoComponent = this->GetComponent<MongoDBComponent>();
        //InnerNetMessageComponent * innerComponent = this->GetComponent<InnerNetMessageComponent>();
        //title.append(fmt::format("   fps:{0}  ", this->mLogicFps));
        //if (innerComponent != nullptr)
        //{
        //    //title.append(fmt::format("rpc:{0}  ", innerComponent->GetWaitCount()));
        //}
        //if (httpComponent != nullptr)
        //{
        //    title.append(fmt::format("http:{0}  ", httpComponent->GetWaitCount()));
        //}
        //if (mongoComponent != nullptr)
        //{
        //    title.append(fmt::format("mogo:{0}  ", mongoComponent->GetWaitCount()));
        //}
		SetConsoleTitle(title.c_str());
	}
#endif
}