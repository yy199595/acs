#include"ThreadTaskAction.h"
#include<Util/TimeHelper.h>
namespace SoEasy
{
	ThreadTaskAction::ThreadTaskAction(Manager * manager, long long id)
	{
		this->mTaskActionId = id;
		this->mBindManager = manager;
		this->mStartTime = TimeHelper::GetMilTimestamp();
	}
	void ThreadTaskAction::NoticeToMainThread()
	{
		if (this->mBindManager)
		{
			this->mBindManager->AddFinishTaskId(this->mTaskActionId);
		}
	}
}