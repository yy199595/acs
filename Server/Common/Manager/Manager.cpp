#include"Manager.h"
#include"LocalActionManager.h"
#include<Core/Applocation.h>
#include<Thread/ThreadPool.h>
#include<Thread/ThreadTaskAction.h>
namespace SoEasy
{
	NetMessageBuffer::NetMessageBuffer(const std::string & address, const shared_ptr<NetWorkPacket> packet)
		:mAddress(address), mMessagePacket(packet)
	{

	}
}
namespace SoEasy
{


	Manager::Manager(const int priority) : mPriority(priority)
	{

	}
	bool Manager::BindFunction(std::string name, LocalAction1 action)
	{
		const size_t pos = name.find_first_of(".");
		if (pos == std::string::npos)
		{
			SayNoDebugError("register error : " << name);
			return false;
		}
		return this->BindFunction(name, make_shared<LocalActionProxy1>(action, name));
	}

	void Manager::AddFinishTask(long long taskId)
	{
		this->mFinishTaskQueue.AddItem(taskId);
	}

	bool Manager::BindFunction(const std::string & name, shared_ptr<LocalActionProxy> actionBox)
	{	
		LocalActionManager * pFunctionManager = this->GetManager<LocalActionManager>();
		SayNoAssertRetFalse_F(pFunctionManager || actionBox);
		const size_t pos = name.find_first_of(".");
		const std::string className = name.substr(0, pos);
		const std::string funcName = name.substr(pos + 1, name.length());
		return pFunctionManager->BindFunction(actionBox);
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