
#include"App.h"
#include"Core/System/System.h"
#include"Lua/Engine/Define.h"
#include"Timer/Timer/ElapsedTimer.h"
#include"Util/File/DirectoryHelper.h"
#include"Proto/Component/ProtoComponent.h"
#include"Server/Component/TextConfigComponent.h"
#include"Server/Component/ThreadComponent.h"
#include"Cluster/Component/LaunchComponent.h"
#ifdef __OS_WIN__
#include<Windows.h>
#endif

using namespace Tendo;
using namespace std::chrono;

namespace Tendo
{

	App::App(int id, const std::string & name) : Server(id, name),
        mThreadId(std::this_thread::get_id()),
				 mStartTime(Helper::Time::NowMilTime())
	{
        this->mLogicFps = 0;
        this->mTickCount = 0;
        this->mIsStartDone = false;
        this->mLogComponent = nullptr;
        this->mTaskComponent = nullptr;
        this->mTimerComponent = nullptr;
		this->mActorComponent = nullptr;
		this->mMessageComponent = nullptr;
		this->mStatus = ServerStatus::Init;
	}

	Actor* App::Random(const std::string& name)
	{
		Actor * actor = this->mActorComponent->RandomActor(name);
		if(actor == nullptr && name == this->Name())
		{
			return this;
		}
		return actor;
	}

	int App::LuaRandom(lua_State* lua)
	{
		if(lua_isstring(lua, 1))
		{
			std::string name(lua_tostring(lua, 1));
			Actor * actor = App::Inst()->Random(name);
			if(actor == nullptr)
			{
				return 0;
			}
			lua_pushinteger(lua, actor->GetActorId());
			return 1;
		}
		lua_pushinteger(lua, App::Inst()->GetActorId());
		return 1;
	}

	bool App::LoadComponent()
	{
		this->mTaskComponent = this->GetOrAddComponent<CoroutineComponent>();
		this->mLogComponent = this->GetOrAddComponent<LogComponent>();
		this->mTimerComponent = this->GetOrAddComponent<TimerComponent>();
		this->mMessageComponent = this->GetOrAddComponent<ProtoComponent>();
		this->mActorComponent = this->GetOrAddComponent<ActorMgrComponent>();

        LOG_CHECK_RET_FALSE(this->AddComponent<TextConfigComponent>());
        LOG_CHECK_RET_FALSE(this->AddComponent<ThreadComponent>());
        LOG_CHECK_RET_FALSE(this->AddComponent<LaunchComponent>());
        std::vector<Component *> components;
        if(this->GetComponents(components) > 0)
        {
			for (Component *component: components)
			{
				if(!component->LateAwake())
				{
					LOG_ERROR(component->GetName() << " LateAwake");
					return false;
				}
			}
        }
		this->mActorComponent->AddServer(this->shared_from_this());
        this->mTaskComponent->Start(&App::StartAllComponent, this);
        return true;
	}

	int App::Run()
    {
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

	void App::Stop(int signum)
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
				this->Name() << " in [" << index << "s] after...");
			this->mTaskComponent->Sleep(1000);
		}

		this->mMainContext->stop();
		this->mLogComponent->SaveAllLog();
		CONSOLE_LOG_INFO("close " << this->Name() << " successful ");
		exit(signum);
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
			component->Start();
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
			complete->Complete();
			this->mTimerComponent->CancelTimer(timerId);
        }
		this->mStatus = ServerStatus::Ready;
		long long t = Helper::Time::NowMilTime() - this->mStartTime;
		LOG_INFO("===== start " << this->Name() << " successful [" << t / 1000.0f << "]s ===========");
    }

#ifdef __OS_WIN__
	void App::UpdateConsoleTitle()
	{       
        std::string title = ServerConfig::Inst()->Name();
        //HttpWebComponent * httpComponent = this->GetComponent<HttpWebComponent>();
        //MongoDBComponent* mongoComponent = this->GetComponent<MongoDBComponent>();
        //DispatchMessageComponent * innerComponent = this->GetComponent<DispatchMessageComponent>();
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