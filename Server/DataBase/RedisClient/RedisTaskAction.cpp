#include"RedisTaskAction.h"
#include<Manager/RedisManager.h>
#include<Coroutine/CoroutineManager.h>
namespace SoEasy
{
	RedisTaskAction::RedisTaskAction(RedisManager * mgr, long long taskId, const std::string & cmd)
		: RedisTaskBase(mgr, taskId, cmd)
	{
		Applocation * app = Applocation::Get();
		SayNoAssertRet_F(this->mCorManager = app->GetManager<CoroutineManager>());
		this->mCoreoutineId = this->mCorManager->GetCurrentCorId();
	}

	void RedisTaskAction::OnTaskFinish()
	{
		SayNoAssertRet_F(this->mCorManager);
		this->mCorManager->Resume(this->mCoreoutineId);
	}
}