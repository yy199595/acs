#include "RedisTaskAction.h"
#include<Manager/RedisManager.h>
namespace SoEasy
{
	RedisTaskAction::RedisTaskAction(RedisManager * mgr, long long taskId, long long corId)
		:ThreadTaskAction(mgr, taskId)
	{
		this->mRedisManager = mgr;
		this->mCoreoutineId = corId;
	}
	void RedisTaskAction::InitCommand(const char * format, va_list command)
	{
		this->mFormat = format;
		this->mCommand = command;
	}

	void RedisTaskAction::InvokeInThreadPool(long long threadId)
	{
		RedisSocket * redisSocket = this->mRedisManager->GetRedisSocket(threadId);
		if (redisSocket == nullptr)
		{
			this->mErrorString = "redis scoket null";
			this->mErrorCode = XCode::RedisSocketIsNull;
			return;
		}
		QuertJsonWritre jsonWrite;
		redisReply * replay = (redisReply*)redisvCommand(redisSocket, this->mFormat.c_str(), this->mCommand);
		if (replay == nullptr)
		{
			this->mErrorString = "redis replay null";
			this->mErrorCode = XCode::RedisReplyIsNull;
			return;
		}
		switch (replay->type)
		{
		case REDIS_REPLY_STATUS:
			this->mErrorCode = XCode::Successful;
			break;
		case REDIS_REPLY_ERROR:
			this->mErrorCode = RedisInvokeFailure;
			this->mErrorString.assign(replay->str, replay->len);
			break;
		case REDIS_REPLY_INTEGER:
			this->mErrorCode = XCode::Successful;
			jsonWrite.Write("data", replay->integer);
			break;
		case REDIS_REPLY_NIL:
			this->mErrorCode = XCode::Successful;
			jsonWrite.Write("data");
			break;
		case REDIS_REPLY_STRING:
			this->mErrorCode = XCode::Successful;
			jsonWrite.Write("data", replay->str, replay->len);
			break;
		case REDIS_REPLY_ARRAY:
			jsonWrite.StartWriteArray("data");
			this->mErrorCode = XCode::Successful;
			for (size_t index = 0; index < replay->elements; index++)
			{
				redisReply * redisData = replay->element[index];
				if (redisData->type == REDIS_REPLY_INTEGER)
				{
					jsonWrite.Write(redisData->integer);
				}
				else if (redisData->type == REDIS_REPLY_STRING)
				{
					jsonWrite.Write(redisData->str, redisData->len);
				}
				else if (redisData->type == REDIS_REPLY_NIL)
				{
					jsonWrite.Write();
				}
			}
			jsonWrite.EndWriteArray();
			break;
		}
		freeReplyObject(replay);
		if (!jsonWrite.Serialization(this->mDocument))
		{
			this->mErrorCode = XCode::RedisJsonParseFail;
			this->mErrorString = "redis cast json failure";
		}
	}
	std::shared_ptr<InvokeResultData> RedisTaskAction::GetInvokeData()
	{
		if (this->mErrorCode != XCode::Successful)
		{
			SayNoDebugError("[redis error] " << this->mErrorString);
		}
		return std::make_shared<InvokeResultData>(mErrorCode, mErrorString, mDocument);
	}
}