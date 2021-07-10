
#include"Applocation.h"
#include"ObjectRegistry.h"
#include<Util/FileHelper.h>
#include<Manager/TimerManager.h>
#include<Manager/ThreadTaskManager.h>
#include<Manager/NetSessionManager.h>
#include<Manager/ActionManager.h>
#include<Coroutine/CoroutineManager.h>
#include<Service/ServiceBase.h>
using namespace SoEasy;
using namespace std::chrono;

namespace SoEasy
{
	Applocation * Applocation::mApplocation = nullptr;

	Applocation::Applocation(const std::string srvName, const std::string configPath)
		: mStartTime(TimeHelper::GetSecTimeStamp()), mConfig(configPath)
	{
		assert(!mApplocation);
		mApplocation = this;
		this->mDelatime = 0;
		this->mLogicTime = 0;
		this->mIsClose = false;
		this->mServerName = srvName;
		this->mLogicRunCount = 0;
		this->mSystemRunCount = 0;
		this->mSrvConfigDirectory = configPath;
		this->mLogHelper = new LogHelper("./Logs", srvName);
	}

	bool Applocation::AddManager(const std::string & name)
	{
		Manager * manager = ObjectRegistry<Manager>::Create(name);
		if (manager == nullptr)
		{
			SayNoDebugError("create " << name << " fail");
			return false;
		}
		auto iter = this->mManagerMap.find(name);
		if (iter != this->mManagerMap.end())
		{
			SayNoDebugError(name << " Has been added");
			return false;
		}
		this->mManagerMap.emplace(name, manager);
		return true;
	}

	void Applocation::GetManagers(std::vector<Manager*>& managers)
	{
		managers.clear();
		auto iter = this->mManagerMap.begin();
		for (; iter != this->mManagerMap.end(); iter++)
		{
			Manager * manager = iter->second;
			managers.push_back(manager);
		}		
	}

	bool Applocation::LoadManager()
	{
		std::vector<std::string> managers;
		if (!mConfig.GetValue("Managers", managers))
		{
			SayNoDebugError("not find field : Managers");
			return false;
		}

		for (size_t index = 0; index < managers.size(); index++)
		{
			const std::string & name = managers[index];
			if (!this->AddManager(name))
			{
				return false;
			}
		}

		this->TryAddManager<TimerManager>();
		this->TryAddManager<ActionManager>();
		this->TryAddManager<NetSessionManager>();
		this->TryAddManager<CoroutineManager>();

		return true;
	}

	bool Applocation::InitManager()
	{
		auto iter = this->mManagerMap.begin();
		for (; iter != this->mManagerMap.end(); iter++)
		{
			Manager * manager = iter->second;
			if (manager->IsActive() == false || manager->OnInit() == false)
			{
				SayNoDebugError("init " << manager->GetTypeName() << " fail");
				return false;
			}
			SayNoDebugLog("init " << manager->GetTypeName() << " successful");
			IFrameUpdate * manager1 = dynamic_cast<IFrameUpdate *>(manager);
			ISystemUpdate * manager2 = dynamic_cast<ISystemUpdate *>(manager);
			ISecondUpdate * manager3 = dynamic_cast<ISecondUpdate *>(manager);

			if (manager1 != nullptr)
			{
				this->mFrameUpdateManagers.push_back(manager1);
				SayNoDebugLog("add " << manager->GetTypeName() << " to FrameUpdateArray");
			}
			if (manager2 != nullptr)
			{
				this->mSystemUpdateManagers.push_back(manager2);
				SayNoDebugLog("add " << manager->GetTypeName() << " to SystemUpdateArray");
			}
			if (manager3 != nullptr)
			{
				this->mSecondUpdateManagers.push_back(manager3);
				SayNoDebugLog("add " << manager->GetTypeName() << " to SecondUpdateArray");
			}
		}

		iter = this->mManagerMap.begin();
		for (; iter != this->mManagerMap.end(); iter++)
		{
			Manager * manager = iter->second;
			manager->OnInitComplete();
		}
		return true;
	}

	int Applocation::Run()
	{
		if (!mConfig.InitConfig())
		{
			Stop();
			return -1;
		}

		if (!this->LoadManager())
		{
			Stop();
			return -2;
		}

		if (!this->InitManager())
		{
			Stop();
			return -3;
		}	
		return this->LogicMainLoop();
	}

	int Applocation::Stop()
	{
		this->mIsClose = true;
		auto iter = this->mManagerMap.begin();
		for (; iter != this->mManagerMap.end(); iter++)
		{
			Manager * manager = iter->second;
			manager->OnDestory();
		}
		this->mManagerMap.clear();
		this->mLogHelper->DropLog();
#ifdef _WIN32
		return getchar();
#endif
		return 0;
	}

	float Applocation::GetMeanFps()
	{
		return 0;
	}

	int Applocation::LogicMainLoop()
	{
		long long interval = 1000 / 33;
		long long fristTimer = TimeHelper::GetMilTimestamp();
		long long startTimer = TimeHelper::GetMilTimestamp();
		long long secondTimer = TimeHelper::GetMilTimestamp();
		this->mLastUpdateTime = TimeHelper::GetMilTimestamp();
		this->mMainLoopStartTime = TimeHelper::GetMilTimestamp();
		int logicFps = 30;
		int systemFps = 100;
		mConfig.GetValue("LogicFps", logicFps);
		mConfig.GetValue("SystemFps", systemFps);

		const long long LogicUpdateInterval = 1000 / logicFps;
		const long long systemUpdateInterval = 1000 / systemFps;
		while (!this->mIsClose)
		{
			startTimer = TimeHelper::GetMilTimestamp();
			std::this_thread::sleep_for(chrono::milliseconds(1));
			for (size_t index = 0; index < this->mSystemUpdateManagers.size(); index++)
			{
				this->mSystemUpdateManagers[index]->OnSystemUpdate();			
			}
			
			if (startTimer - mLastUpdateTime >= LogicUpdateInterval)
			{
				this->mLogicRunCount++;
				for (size_t index = 0; index < this->mFrameUpdateManagers.size(); index++)
				{
					this->mFrameUpdateManagers[index]->OnFrameUpdate(this->mDelatime);
				}				

				for (size_t index = 0; index < this->mLastFrameUpdateManager.size(); index++)
				{
					this->mLastFrameUpdateManager[index]->OnLastFrameUpdate();
				}

				startTimer = mLastUpdateTime = TimeHelper::GetMilTimestamp();
			}

			if (startTimer - secondTimer >= 1000)
			{
				for (size_t index = 0; index < this->mSecondUpdateManagers.size(); index++)
				{
					this->mSecondUpdateManagers[index]->OnSecondUpdate();
				}
				this->UpdateConsoleTitle();
				secondTimer = TimeHelper::GetMilTimestamp();
			}		
		}
		return this->Stop();
	}


	bool Applocation::GetTypeName(size_t hash, std::string & name)
	{
		return ObjectRegistry<Manager>::GetTypeName(hash, name);
	}

	void Applocation::UpdateConsoleTitle()
	{
		long long nowTime = TimeHelper::GetMilTimestamp();
		float seconds = (nowTime - this->mMainLoopStartTime) / 1000.0f;
		this->mSystemFps = this->mSystemRunCount / seconds;
		this->mLogicFps = this->mLogicRunCount / seconds;
#ifdef _WIN32
		std::string contitle = this->mServerName + " system:" + 
			std::to_string(this->mSystemFps)+ "  logic:" + std::to_string(this->mLogicFps);
		SetConsoleTitle(contitle.c_str());
#endif
		
	}
}