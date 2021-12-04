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
		this->mDeadloop = 30;
        this->mIsClose = false;	
		App::Get().GetConfig().GetValue("Deadloop", this->mDeadloop);
		this->mTaskComponent = this->GetComponent<TaskPoolComponent>();
        this->mThread = new std::thread(&MonitorComponent::Update, this);     
        return true;
    }

    void MonitorComponent::Update()
    {
        std::vector<const IThread *> threads;
        this->mTaskComponent->GetAllThread(threads);
        while (!this->mIsClose)
        {
            std::this_thread::sleep_for(std::chrono::seconds(this->mDeadloop));
            long long nowTime = TimeHelper::GetSecTimeStamp();
            for (const IThread *taskThread: threads)
            {
                if (taskThread->IsWork() && nowTime - taskThread->GetLastOperTime() >= this->mDeadloop)
                {
                    const std::string &name = taskThread->GetName();
                    LOG_FATAL(name << " thread no response for a long time " << taskThread->GetThreadId());
                }
            }
        }
    }
}