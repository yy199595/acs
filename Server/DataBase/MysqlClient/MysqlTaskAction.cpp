#include"MysqlTaskAction.h"
#include<Manager/MysqlManager.h>
#include<Coroutine/CoroutineManager.h>
#include<QueryResult/InvokeResultData.h>
namespace SoEasy
{
	MysqlTaskAction::MysqlTaskAction(MysqlManager * mgr, long long id, const std::string & db, const std::string & sql, CoroutineManager * corMgr)
		: MysqlTaskBase(mgr, id, db), mSqlCommand(sql)
	{
		this->mCoroutineMgr = corMgr;
		this->mCoroutineId = corMgr->GetCurrentCorId();
		SayNoAssertRet_F(this->mCoroutineId != 0)
	}

	void MysqlTaskAction::OnTaskFinish()
	{
		SayNoAssertRet_F(this->mCoroutineMgr);
		this->mCoroutineMgr->Resume(this->mCoroutineId);
	}

	std::shared_ptr<InvokeResultData> MysqlTaskAction::GetInvokeData()
	{	
		XCode code = this->GetErrorCode();
		const std::string & error = this->GetErrorStr();
		return std::make_shared<InvokeResultData>(code, error , mDocument);
	}
}