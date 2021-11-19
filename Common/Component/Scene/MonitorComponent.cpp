//
// Created by zmhy0073 on 2021/10/29.
//


#include"MonitorComponent.h"
#include"Util/TimeHelper.h"
#include"Core/App.h"
#include"Scene/TaskPoolComponent.h"
namespace GameKeeper
{
    bool MonitorComponent::Awake()
    {
        this->mIsClose = false;
        const ServerConfig & serverConfig = App::Get().GetConfig();
        this->mThread = new std::thread(&MonitorComponent::Update, this);
        GKAssertRetFalse_F(this->mTaskComponent = this->GetComponent<TaskPoolComponent>());
        return true;
    }

    void MonitorComponent::Update()
    {
        std::vector<const IThread *> threads;
        this->mTaskComponent->GetAllThread(threads);
        this->mStartTime = TimeHelper::GetSecTimeStamp();
        while (!this->mIsClose)
        {
            long long nowTime = TimeHelper::GetSecTimeStamp();
            std::this_thread::sleep_for(std::chrono::seconds(1));
            this->mStartTime += 1;
            for (const IThread *taskThread: threads)
            {
                if (!taskThread->IsWork())
                {
                    continue;
                }
                if (nowTime - taskThread->GetLastOperTime() >= 5)
                {
                    const std::string &name = taskThread->GetName();
                    GKDebugFatal(name << " thread no response for a long time");
                }
            }
        }
    }
}