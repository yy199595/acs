﻿#include"App.h"

#ifndef __OS_WIN__
#include<csignal>
#endif

#include "Lua/Engine/Define.h"
#include "Core/System/System.h"
#include "Server/Config/ServerConfig.h"
#include "Timer/Timer/ElapsedTimer.h"
#include "Util/File/DirectoryHelper.h"
#include "Proto/Component/ProtoComponent.h"
#include "Server/Component/ThreadComponent.h"
#include "Cluster/Component/LaunchComponent.h"
#include "Log/Component/LoggerComponent.h"
#include "Timer/Component/TimerComponent.h"
#include "Cluster/Config/ClusterConfig.h"
#include "Config/Base/LangConfig.h"

#ifdef __ENABLE_OPEN_SSL__

#include "Auth/Aes/Aes.h"
#include "Auth/Jwt/Jwt.h"
#else

#include "Util/Crypt/Mask.h"
#include "Proto/Bson/base64.h"

#endif

#ifndef __DEBUG__
#include "Http/Component/NotifyComponent.h"
#endif

namespace acs
{
	App::App(int id, ServerConfig& config) :
			Server(id, config.Name()), mSignal(mContext),
			mContext(1), mStartTime(help::Time::NowMil()), mConfig(config)
	{
		this->mLogicFps = 0;
		this->mTickCount = 0;
		this->mGuidIndex = 0;
		this->mLastGuidTime = 0;
		this->mActor = nullptr;
		this->mProto = nullptr;
		this->mStartMemory = 0;
		this->mCoroutine = nullptr;
		this->mStatus = ServerStatus::Init;
#ifdef __OS_WIN__
		Debug::Init();
#endif
#ifdef __ENABLE_OPEN_SSL__
#if OPENSSL_VERSION_NUMBER < 0x10100000L
		OpenSSL_add_all_algorithms();
		ERR_load_crypto_strings();
#else
		OPENSSL_init_crypto(OPENSSL_INIT_LOAD_CONFIG, nullptr);
#endif
#endif

#ifdef __ENABLE_OPEN_WOLF_SSL__

#endif
		this->mNextNewDayTime = help::Time::GetNewTime(1);
	}

	bool App::LoadComponent()
	{
		std::string path, cluster;
		if (!this->mConfig.GetPath("cluster", path)) //加载集群配置
		{
			return false;
		}
		TextConfig* config = new ClusterConfig();
		if (!config->LoadConfig(path))
		{
			return false;
		}

		this->AddComponent<ThreadComponent>();
		this->AddComponent<LoggerComponent>();
		LOG_CHECK_RET_FALSE(this->AddComponent<TimerComponent>());
		LOG_CHECK_RET_FALSE(this->AddComponent<LaunchComponent>());
		LOG_CHECK_RET_FALSE(this->AddComponent<CoroutineComponent>());

		this->mActor = this->GetComponent<ActorComponent>();
		this->mProto = this->GetComponent<ProtoComponent>();
		this->mCoroutine = this->GetComponent<CoroutineComponent>();

		LOG_CHECK_RET_FALSE(this->InitComponent());
#ifndef __OS_WIN__
		this->mSignal.add(SIGINT);
		this->mSignal.add(SIGTERM);
		this->mSignal.async_wait([this](const asio::error_code& code, int signal)
		{
			this->mCoroutine->Start(&App::Stop, this);
		});
#endif
		this->mCoroutine->Start(&App::StartAllComponent, this);
		if (this->mActor == nullptr)
		{
			return true;
		}
		LOG_CHECK_RET_FALSE(this->LateAwake())
		return this->mActor->AddGroup(this->mConfig.Name(), this->GetId());
	}

	bool App::LoadLang() const
	{
		std::string path;
		if (!this->mConfig.GetPath("lang", path))
		{
			return true;
		}
		return (new LangConfig())->LoadConfig(path);
	}

	bool App::Hotfix()
	{
		std::vector<IHotfix*> hotfixComponents;
		this->GetComponents<IHotfix>(hotfixComponents);
		for (IHotfix* hotfixComponent: hotfixComponents)
		{
			if (!hotfixComponent->OnHotFix())
			{
				Component* component = dynamic_cast<Component*>(hotfixComponent);
				LOG_ERROR("{} invoke hotfix failure", component->GetName());
				return false;
			}
		}
		return true;
	}

	unsigned int App::StartCoroutine(std::function<void()>&& func)
	{
		return this->mCoroutine->Start(std::move(func));
	}

	bool App::InitComponent()
	{
		std::vector<Component*> components;
		this->GetComponents(components);
		for (Component* component: components)
		{
#ifdef __DEBUG__
			//timer::ElapsedTimer timer;
			//LOG_DEBUG("start component => {}", component->GetName())
#endif
			if (!component->LateAwake())
			{
				LOG_ERROR("{} LateAwake fail", component->GetName());
				return false;
			}
#ifdef __DEBUG__
			//LOG_DEBUG("[{}ms] [{}.LateAwake] ok", timer.GetMs(), component->GetName());
#endif
		}
		return true;
	}

	void App::Sleep(int ms)
	{
		this->mCoroutine->Sleep(ms);
	}

	int App::Run() noexcept
	{
		srand(help::Time::NowMil());
		if (!this->LoadLang())
		{
			return XServerCode::ConfError;
		}
		if (!this->LoadComponent())
		{
#ifdef __OS_WIN__
			return getchar();
#else
			return XServerCode::InitError;
#endif
		}

		long long logicStartTime = 0;
		long long logicSecondTime = help::Time::NowMil();
		long long logicLastUpdateTime = help::Time::NowMil();

		std::vector<IFrameUpdate*> frameUpdateComponents;
		std::vector<ISystemUpdate*> systemUpdateComponents;
		std::vector<ISecondUpdate*> secondUpdateComponents;
		std::vector<ILastFrameUpdate*> lastFrameUpdateComponents;
		this->GetComponents<IFrameUpdate>(frameUpdateComponents);
		this->GetComponents<ISystemUpdate>(systemUpdateComponents);
		this->GetComponents<ISecondUpdate>(secondUpdateComponents);
		this->GetComponents<ILastFrameUpdate>(lastFrameUpdateComponents);


		int fps = 15;
		Asio::Code code;
		long long logicRunCount = 0;
		Asio::ContextWork work(this->mContext);
		std::unique_ptr<json::r::Value> jsonObject;
		if (this->mConfig.Get("core", jsonObject))
		{
			jsonObject->Get("fps", fps);
		}
		std::chrono::milliseconds sleepTime(1);
		long long logicUpdateInterval = 1000 / fps;

		while (!this->mContext.stopped())
		{
			logicRunCount++;
			this->mContext.poll(code);
			for (ISystemUpdate* component: systemUpdateComponents)
			{
				component->OnSystemUpdate();
			}
			if (this->mStatus >= ServerStatus::Start && this->mStatus < ServerStatus::Closing)
			{
				logicStartTime = help::Time::NowMil();
				if (logicStartTime - logicLastUpdateTime >= logicUpdateInterval)
				{
					for (IFrameUpdate* component: frameUpdateComponents)
					{
						component->OnFrameUpdate(logicStartTime);
					}
					long long nowTime = help::Time::NowMil();
					logicUpdateInterval = (1000 / fps) - (nowTime - logicStartTime);

					if (logicStartTime - logicSecondTime >= 1000)
					{
#ifdef __OS_WIN__
						os::SystemInfo systemInfo;
						constexpr double MB = 1024 * 1024.0f;
						os::System::GetSystemInfo(systemInfo);
						double mb = (double)(systemInfo.use_memory - this->mStartMemory) / MB;
						SetConsoleTitle(fmt::format("{:.3f}MB", mb).c_str());
#endif
#if defined(__OS_WIN__) || defined(__OS_MAC__)
						this->Hotfix();
#endif
						this->mTickCount++;
						long long costTime = nowTime - logicSecondTime;
						float seconds = (float)costTime / 1000.0f;
						this->mLogicFps = (float)logicRunCount / seconds;
						for (ISecondUpdate* component: secondUpdateComponents)
						{
							component->OnSecondUpdate(this->mTickCount);
						}

						if ((nowTime / 1000) >= this->mNextNewDayTime)
						{
							std::vector<ISystemNewDay*> systemNewDays;
							this->GetComponents<ISystemNewDay>(systemNewDays);
							for (ISystemNewDay* systemNewDay: systemNewDays)
							{
								systemNewDay->OnNewDay();
							}
							this->mNextNewDayTime = help::Time::GetNewTime(1);
						}
						logicRunCount = 0;
						logicSecondTime = help::Time::NowMil();
					}

					for (ILastFrameUpdate* component: lastFrameUpdateComponents)
					{
						component->OnLastFrameUpdate(logicStartTime);
					}
					logicLastUpdateTime = help::Time::NowMil();
				}

			}
			std::this_thread::sleep_for(sleepTime);
		}
		printf("========== close server ==========\n");
		return XServerCode::Ok;
	}

	long long App::MakeGuid()
	{
		long long nowTime = help::Time::NowSec();
		if (nowTime != this->mLastGuidTime)
		{
			this->mGuidIndex = 0;
			this->mLastGuidTime = nowTime;
		}
		int index = this->mGuidIndex++;
		if (index >= std::numeric_limits<unsigned short>::max())
		{
			nowTime++;
			index = 0;
			this->mGuidIndex = 0;
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		int serverId = this->GetSrvId();
		return nowTime << 31 | (int)serverId << 16 | index;
	}

	std::string App::NewUuid()
	{
		long long guid = this->MakeGuid();
		return std::to_string(guid);
	}


	std::string App::Sign(json::w::Document& document)
	{
		std::string data;
		document.Encode(&data);
		const std::string& key = this->mConfig.GetSecretKey();
#ifdef __ENABLE_OPEN_SSL__
		return jwt::Create(data, key);
#else
		help::Mask::Encode(data, key);
		return _bson::base64::encode(data);
#endif
	}

	bool App::DecodeSign(const std::string& sign, json::r::Document& document)
	{
		const std::string& key = this->mConfig.GetSecretKey();
#ifdef __ENABLE_OPEN_SSL__
		std::string data;
		if (!jwt::Verify(sign, key, data))
		{
			return false;
		}
		return document.Decode(data);
#else
		std::string output = _bson::base64::decode(sign);
		help::Mask::Decode(output, key);
		return document.Decode(output);
#endif
	}


	void App::Stop()
	{
		if (this->mStatus == ServerStatus::Closing)
		{
			return;
		}
		this->mStatus = ServerStatus::Closing;
#ifdef __DEBUG__
		CONSOLE_LOG_ERROR("start close {}", this->Name());
#else
		long long t1 = help::Time::NowMil();
#endif
		std::vector<IAppStop*> stopComponent;
		this->GetComponents<IAppStop>(stopComponent);

		for (IAppStop* component: stopComponent)
		{
			component->OnAppStop(); //保存数据
		}

		std::vector<IDestroy*> components;
		this->GetComponents<IDestroy>(components);
		std::reverse(components.begin(), components.end());
		for (size_t index = 0; index < components.size(); index++)
		{
			IDestroy* nextComponent = nullptr;
			IDestroy* component = components.at(index);
			if (index < components.size() - 1)
			{
				nextComponent = components.at(index + 1);
			}
			std::string name2("null");
			float process = (index + 1) / (float)components.size();
			const std::string& name1 = dynamic_cast<Component*>(component)->GetName();
			if (nextComponent != nullptr)
			{
				name2 = dynamic_cast<Component*>(nextComponent)->GetName();
			}
			CONSOLE_LOG_INFO("[{:.2f}%] close {} => {}", process * 100, name1, name2);
			component->OnDestroy();
		}
		this->mContext.stop();
	}

	void App::StartAllComponent()
	{
		std::vector<IStart*> startComponents;
		this->GetComponents<IStart>(startComponents);
		TimerComponent* timerComponent = this->GetComponent<TimerComponent>();
		for (IStart* component: startComponents)
		{
			timer::ElapsedTimer timer;
			const std::string& name = dynamic_cast<Component*>(component)->GetName();
			const long long timeId = timerComponent->DelayCall(10 * 1000, [name]()
			{
				LOG_FATAL("{} start time out", name);
			});
			component->OnStart();
			timerComponent->CancelTimer(timeId);
			if (timer.GetMs() > 100)
			{
				LOG_DEBUG("[{}ms] => {}.Start", timer.GetMs(), name);
			}
		}
		startComponents.clear();
		this->mStatus = ServerStatus::Start; //开始帧循环
		std::vector<IComplete*> completeComponents;
		this->GetComponents<IComplete>(completeComponents);
		for (IComplete* complete: completeComponents)
		{
			timer::ElapsedTimer timer;
			Component* component = dynamic_cast<Component*>(complete);
			const std::string& name = dynamic_cast<Component*>(component)->GetName();
			const long long timerId = timerComponent->DelayCall(1000 * 10, [component]()
			{
				LOG_ERROR("{0}.Complete call timeout", component->GetName());
			});
			complete->OnComplete();
			if (timer.GetMs() > 100)
			{
				LOG_DEBUG("[{}ms] => {}.Complete", timer.GetMs(), name);
			}
			timerComponent->CancelTimer(timerId);
		}
		os::SystemInfo systemInfo;
		os::System::GetSystemInfo(systemInfo);

		completeComponents.clear();
		this->mStatus = ServerStatus::Ready;
		this->mStartMemory = systemInfo.use_memory;
		long long t = help::Time::NowMil() - this->mStartTime;
		LOG_INFO("  ===== start {} ok [{:.3f}s] =======", this->Name(), t / 1000.0f);
	}
}