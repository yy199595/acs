#include"Manager.h"
#include"ActionManager.h"
#include<Core/Applocation.h>
namespace SoEasy
{
	Manager::Manager(const int priority) : mPriority(priority)
	{

	}
	bool Manager::BindFunction(std::string name, NetWorkAction1 action)
	{
		const size_t pos = name.find_first_of(".");
		if (pos == std::string::npos)
		{
			SayNoDebugError("register error : " << name);
			return false;
		}
		NetWorkActionBox * actionBox = new NetWorkActionBox1(action, name);
		return this->BindFunction(name, actionBox);
	}

	void Manager::AddFinishTaskId(long long taskId)
	{
		this->mFinishTaskQueue.AddItem(taskId);
	}



	bool Manager::BindFunction(const std::string & name, NetWorkActionBox * actionBox)
	{	
		ActionManager * pFunctionManager = this->GetManager<ActionManager>();
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