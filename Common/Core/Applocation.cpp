
#include "Applocation.h"
#include <Object/ObjectRegistry.h>
#include <Coroutine/CoroutineManager.h>
#include <Manager/ActionManager.h>
#include <Manager/NetSessionManager.h>
#include <Timer/TimerManager.h>
#include <Util/FileHelper.h>
#include <Manager/ProtocolManager.h>

using namespace Sentry;
using namespace std::chrono;

namespace Sentry
{
    Applocation *Applocation::mApplocation = nullptr;

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
        this->mNetWorkThread = nullptr;
        this->mSrvConfigDirectory = configPath;
        this->mLogHelper = new LogHelper("./Logs", srvName);
    }

    bool Applocation::AddManager(const std::string &name)
    {
        Manager *manager = ObjectRegistry<Manager>::Create(name);
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

    void Applocation::GetManagers(std::vector<Manager *> &managers)
    {
        managers.clear();
        auto iter = this->mManagerMap.begin();
        for (; iter != this->mManagerMap.end(); iter++)
        {
            Manager *manager = iter->second;
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
            const std::string &name = managers[index];
            if (!this->AddManager(name))
            {
                return false;
            }
        }

        this->TryAddManager<TimerManager>();
        this->TryAddManager<ActionManager>();
        this->TryAddManager<ProtocolManager>();
        this->TryAddManager<CoroutineManager>();
        this->TryAddManager<NetSessionManager>();
        return true;
    }

    bool Applocation::InitManager()
    {

        for (auto iter = this->mManagerMap.begin(); iter != this->mManagerMap.end(); iter++)
        {
            Manager *manager = iter->second;
            if (manager->IsActive() == false || manager->OnInit() == false)
            {
                SayNoDebugError("init " << manager->GetTypeName() << " fail");
                return false;
            }
            IFrameUpdate *manager1 = dynamic_cast<IFrameUpdate *>(manager);
            ISystemUpdate *manager2 = dynamic_cast<ISystemUpdate *>(manager);
            ISecondUpdate *manager3 = dynamic_cast<ISecondUpdate *>(manager);
            INetSystemUpdate *manager4 = dynamic_cast<INetSystemUpdate *>(manager);
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
            if (manager4 != nullptr)
            {
                this->mNetSystemUpdateManagers.push_back(manager4);
                SayNoDebugLog("add " << manager->GetTypeName() << " to NetSystemUpdateArray");
            }
        }

        for (auto iter = this->mManagerMap.begin(); iter != this->mManagerMap.end(); iter++)
        {
            Manager *manager = iter->second;
            manager->OnInitComplete();
        }


        this->mNetWorkThread = new std::thread(std::bind(BIND_THIS_ACTION_0(Applocation::NetworkLoop)));

        if (this->mNetWorkThread != nullptr)
        {
            this->mMainThreadId = std::this_thread::get_id();
            this->mNetWorkThreadId = this->mNetWorkThread->get_id();

            SayNoDebugLog("=====  start " << this->mServerName << " successful  ========");
            SayNoDebugInfo(
                    "main thread : [" << this->mMainThreadId << "]  " << "net thread : [" << this->mNetWorkThreadId
                                      << "]");
            return true;
        }
        return false;
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
            Manager *manager = iter->second;
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
        long long startTimer = TimeHelper::GetMilTimestamp();
        long long secondTimer = TimeHelper::GetMilTimestamp();
        this->mLastUpdateTime = TimeHelper::GetMilTimestamp();
        this->mMainLoopStartTime = TimeHelper::GetMilTimestamp();
        int logicFps = 30;
        int systemFps = 100;
        mConfig.GetValue("LogicFps", logicFps);
        mConfig.GetValue("SystemFps", systemFps);

        std::chrono::milliseconds time(1);
        const long long LogicUpdateInterval = 1000 / logicFps;
        while (!this->mIsClose)
        {
            std::this_thread::sleep_for(time);
            startTimer = TimeHelper::GetMilTimestamp();
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

    void Applocation::NetworkLoop()
    {
        std::chrono::milliseconds time(1);
        while (this->mIsClose == false)
        {
            mAsioContext.poll();
            std::this_thread::sleep_for(time);
            for (size_t index = 0; index < this->mNetSystemUpdateManagers.size(); index++)
            {
                INetSystemUpdate *manager = this->mNetSystemUpdateManagers[index];
                manager->OnNetSystemUpdate(this->mAsioContext);
            }
        }
    }


    bool Applocation::GetTypeName(size_t hash, std::string &name)
    {
        return ObjectRegistry<Manager>::GetTypeName(hash, name);
    }

    void Applocation::UpdateConsoleTitle()
    {
        long long nowTime = TimeHelper::GetMilTimestamp();
        float seconds = (nowTime - this->mMainLoopStartTime) / 1000.0f;
        this->mLogicFps = this->mLogicRunCount / seconds;
#ifdef _WIN32
        char buffer[100] = {0};
        sprintf_s(buffer, "%s fps:%f", this->mServerName.c_str(), this->mLogicFps);
        SetConsoleTitle(buffer);
#endif
    }
}// namespace Sentry