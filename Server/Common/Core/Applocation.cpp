
#include"Applocation.h"
#include"ManagerFactory.h"
#include<Util/FileHelper.h>
#include<Thread/ThreadPool.h>
#include<Manager/TimerManager.h>
#include<Manager/NetWorkManager.h>
#include<Manager/ActionManager.h>
#include<Coroutine/CoroutineManager.h>
using namespace SoEasy;
using namespace std::chrono;

namespace SoEasy
{
	Applocation * Applocation::mApplocation = nullptr;

	Applocation::Applocation(const std::string srvName, ManagerFactory & factory, const std::string configPath)
		: mStartTime(TimeHelper::GetSecTimeStamp()), mConfig(configPath), mManagerFactory(factory)
	{
		assert(!mApplocation);
		mApplocation = this;
		this->mDelatime = 0;
		this->mLogicTime = 0;
		this->mIsClose = false;
		this->mThreadPool = nullptr;
		this->mCurrentManager = nullptr;
		this->mSrvConfigDirectory = configPath;
		this->mAsioContext = new AsioContext();
		this->mAsioWork = new AsioWork(*mAsioContext);
		this->mLogHelper = new LogHelper("./Logs", srvName);
	}

	bool Applocation::AddManager(const std::string & name)
	{
		Manager * manager = mManagerFactory.Create(name);
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
		manager->Init(this, name);
		this->mManagerMap.emplace(name, manager);
		this->mSortManagers.emplace_back(manager);
		return true;
	}

	bool Applocation::LoadManager()
	{
		std::vector<std::string> managers;
		if (!mConfig.GetValue("Managers", managers))
		{
			SayNoDebugError("not find field : Managers");
			return false;
		}
		unsigned int count = 0;
		if (mConfig.GetValue("ThreadCount", count))
		{
			this->mThreadPool = new ThreadPool(count);
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
		this->TryAddManager<AddressManager>();
		this->TryAddManager<NetWorkManager>();
		this->TryAddManager<CoroutineManager>();

		std::sort(this->mSortManagers.begin(), this->mSortManagers.end(),
			[](Manager * m1, Manager * m2)->bool
		{
			return m1->GetPriority() < m2->GetPriority();
		});
		return true;
	}

	bool Applocation::InitManager()
	{
		for (size_t index = 0; index < this->mSortManagers.size(); index++)
		{
			this->mCurrentManager = this->mSortManagers[index];
			if (!this->mCurrentManager->IsActive() || !this->mCurrentManager->OnInit())
			{
				SayNoDebugError("init " << this->mCurrentManager->GetTypeName() << " fail");
				return false;
			}
			SayNoDebugLog("init " << this->mCurrentManager->GetTypeName() << " successful");
		}

		for (size_t index = 0; index < this->mSortManagers.size(); index++)
		{
			this->mCurrentManager = this->mSortManagers[index];
			this->mCurrentManager->OnInitComplete();
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

		this->mNetThread = new std::thread([this]()
		{
			this->mAsioContext->run();
		});
		this->mNetThread->detach();

		return this->LogicMainLoop();
	}

	int Applocation::Stop()
	{
		this->mIsClose = true;
		this->mAsioContext->stop();
		for (size_t index = 0; index < this->mSortManagers.size(); index++)
		{
			this->mCurrentManager = this->mSortManagers[index];
			this->mCurrentManager->OnDestory();
		}
		this->mManagerMap.clear();
		this->mSortManagers.clear();
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
		this->mLastSystemTime = TimeHelper::GetMilTimestamp();
		int logicFps = 30;
		int systemFps = 100;
		mConfig.GetValue("LogicFps", logicFps);
		mConfig.GetValue("SystemFps", systemFps);

		const long long LogicUpdateInterval = 1000 / logicFps;
		const long long systemUpdateInterval = 1000 / systemFps;

		while (!this->mIsClose)
		{
			startTimer = TimeHelper::GetMilTimestamp();
			if (startTimer - mLastSystemTime >= systemUpdateInterval)
			{
				for (size_t index = 0; index < this->mSortManagers.size(); index++)
				{
					this->mCurrentManager = this->mSortManagers[index];
					this->mCurrentManager->OnSystemUpdate();
				}
				startTimer = mLastSystemTime = TimeHelper::GetMilTimestamp();
			}

			if (startTimer - mLastUpdateTime >= LogicUpdateInterval)
			{
				for (size_t index = 0; index < this->mSortManagers.size(); index++)
				{
					this->mCurrentManager = this->mSortManagers[index];
					this->mCurrentManager->OnFrameUpdate(this->mDelatime);
				}
				startTimer = mLastUpdateTime = TimeHelper::GetMilTimestamp();
			}

			if (startTimer - secondTimer >= 1000)
			{
				for (size_t index = 0; index < this->mSortManagers.size(); index++)
				{
					this->mCurrentManager = this->mSortManagers[index];
					this->mCurrentManager->OnSecondUpdate();
				}
				this->UpdateConsoleTitle();
				secondTimer = TimeHelper::GetMilTimestamp();
			}
			std::this_thread::sleep_for(chrono::milliseconds(1));
		}
		return this->Stop();
	}

	void Applocation::UpdateConsoleTitle()
	{
#ifdef _WIN32
		
#endif
	}
}