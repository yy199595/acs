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

	bool Manager::StartTaskAction(shared_ptr<ThreadTaskAction> taskAction)
	{
		if (taskAction == nullptr)
		{
			return false;
		}
		ThreadPool * threadPool = this->GetApp()->GetThreadPool();
		SayNoAssertRetFalse_F(threadPool);
		long long taskId = taskAction->GetTaskId();
		auto iter = this->mThreadTaskMap.find(taskId);
		if (iter != this->mThreadTaskMap.end())
		{
			SayNoDebugError("task exits " << taskId);
			return false;
		}
		if (!threadPool->StartTaskAction(taskAction))
		{
			this->mWaitDispatchTasks.push(taskAction);
		}
		this->mThreadTaskMap.emplace(taskId, taskAction);
		return true;
	}
}