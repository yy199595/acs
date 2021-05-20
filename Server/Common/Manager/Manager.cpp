#include"Manager.h"
#include"ActionManager.h"
#include<Core/Applocation.h>
#include<Thread/ThreadPool.h>
#include<Thread/ThreadTaskAction.h>

namespace SoEasy
{

	
	NetMessageBuffer::NetMessageBuffer(const std::string & address, const SharedPacket packet)
		:mAddress(address), mMessagePacket(packet)
	{

	}
	


	Manager::Manager(const int priority) : mPriority(priority)
	{

	}
	
	void Manager::AddFinishTask(long long taskId)
	{
		this->mFinishTaskQueue.AddItem(taskId);
	}

	void Manager::ForeachManagers(std::function<bool(Manager*)> action)
	{
		std::vector<Manager *> managers;
		this->GetApp()->GetManagers(managers);
		for (size_t index = 0; index < managers.size(); index++)
		{
			Manager * manager = managers[index];
			if (action(manager) == false)
			{
				break;
			}
		}
	}

	void Manager::OnSystemUpdate()
	{	
		long long taskId = 0;
		this->mFinishTaskQueue.SwapQueueData();
		while (this->mFinishTaskQueue.PopItem(taskId))
		{
			auto iter = this->mThreadTaskMap.find(taskId);
			if (iter != this->mThreadTaskMap.end())
			{
				SharedThreadTask taskAction = iter->second;
				if (taskAction != nullptr)
				{
					taskAction->OnTaskFinish();
					this->OnTaskFinish(taskAction);
				}				
				this->mThreadTaskMap.erase(iter);
			}			
		}
	}

	bool Manager::StartTaskAction(shared_ptr<ThreadTaskAction> taskAction)
	{
		ThreadPool * threadPool = this->GetApp()->GetThreadPool();
		SayNoAssertRetFalse_F(threadPool);
		long long taskId = taskAction->GetTaskId();
		auto iter = this->mThreadTaskMap.find(taskId);
		if (iter == this->mThreadTaskMap.end())
		{
			if (threadPool->StartTaskAction(taskAction))
			{
				this->mThreadTaskMap.emplace(taskId, taskAction);
				return true;
			}
		}
		return false;
	}
}