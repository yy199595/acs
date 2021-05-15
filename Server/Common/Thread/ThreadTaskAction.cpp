#include"ThreadTaskAction.h"
#include<Manager/Manager.h>
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
		SayNoAssertRet_F(this->mBindManager);
		this->mBindManager->AddFinishTask(this->mTaskActionId);
	}
}