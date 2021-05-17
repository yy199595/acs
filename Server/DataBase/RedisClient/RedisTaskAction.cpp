#include"RedisTaskAction.h"
#include<Manager/RedisManager.h>
#include<Coroutine/CoroutineManager.h>
namespace SoEasy
{
	RedisTaskAction::RedisTaskAction(RedisManager * mgr, long long taskId, const std::string & cmd, CoroutineManager * corMgr)
		: RedisTaskBase(mgr, taskId, cmd)
	{
		this->mCoroutineManager = corMgr;
		this->mCoreoutineId = corMgr->GetCurrentCorId();
	}

	void RedisTaskAction::OnTaskFinish()
	{
		SayNoAssertRet_F(this->mCoroutineManager);
		if (this->GetErrorCode() != XCode::Successful)
		{
			SayNoDebugError("[redis error ]" << this->GetErrorStr());
		}
		this->mCoroutineManager->Resume(this->mCoreoutineId);
	}

	void RedisTaskAction::OnQueryFinish(QuertJsonWritre & jsonWriter)
	{		
		SayNoAssertRet_F(jsonWriter.Serialization(this->mDocument));
	}

	std::shared_ptr<InvokeResultData> RedisTaskAction::GetInvokeData()
	{
		XCode code = this->GetErrorCode();
		const std::string & error = this->GetErrorStr();
		return std::make_shared<InvokeResultData>(code, error, mDocument);
	}
}