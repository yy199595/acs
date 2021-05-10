#include"Manager.h"
#include"LocalActionManager.h"
#include<Core/Applocation.h>
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
		LocalActionProxy * actionBox = new LocalActionProxy1(action, name);
		return this->BindFunction(name, actionBox);
	}

	void Manager::AddFinishTaskId(long long taskId)
	{
		this->mFinishTaskQueue.AddItem(taskId);
	}



	bool Manager::BindFunction(const std::string & name, LocalActionProxy * actionBox)
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
		
	}

	void Manager::OnSystemUpdate()
	{	
		long long taskId = 0;
		this->mFinishTaskQueue.SwapQueueData();
		while (this->mFinishTaskQueue.PopItem(taskId))
		{
			this->OnTaskFinish(taskId);
		}
	}
}