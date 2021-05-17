#include"RedisTaskBase.h"
#include<Manager/RedisManager.h>
namespace SoEasy
{

	RedisTaskBase::RedisTaskBase(RedisManager * mgr, long long taskId, const std::string & cmd)
		: ThreadTaskAction(mgr, taskId)
	{
		this->mRedisManager = mgr;
		this->mCommand.push_back(cmd);
	}

	void RedisTaskBase::InvokeInThreadPool(long long threadId)
	{
		QuertJsonWritre jsonWrite;
		RedisSocket * redisSocket = this->mRedisManager->GetRedisSocket(threadId);
		if (redisSocket == nullptr)
		{
			this->mErrorStr = "redis scoket null";
			this->mErrorCode = XCode::RedisSocketIsNull;
			jsonWrite.Write("code", (long long)this->mErrorCode);
			jsonWrite.Write("error", this->mErrorStr.c_str(), this->mErrorStr.size());
			this->OnQueryFinish(jsonWrite);
			return;
		}

		for (size_t index = 0; index < this->mCommand.size(); index++)
		{
			this->mArgvArray[index] = this->mCommand[index].c_str();
			this->mArgvSizeArray[index] = this->mCommand[index].size();
		}
		const int size = (int)this->mCommand.size();
		redisReply * replay = (redisReply*)redisCommandArgv(redisSocket, size, this->mArgvArray, this->mArgvSizeArray);

		//redisReply * replay = (redisReply*)redisvCommand(redisSocket, this->mFormat.c_str(), this->mCommand);
		if (replay == nullptr)
		{
			this->mErrorStr = "redis replay null";
			this->mErrorCode = XCode::RedisReplyIsNull;
			jsonWrite.Write("code", (long long)this->mErrorCode);
			jsonWrite.Write("error", this->mErrorStr.c_str(), this->mErrorStr.size());
			this->OnQueryFinish(jsonWrite);
			return;
		}
		switch (replay->type)
		{
		case REDIS_REPLY_STATUS:
			this->mErrorCode = XCode::Successful;
			break;
		case REDIS_REPLY_ERROR:
			this->mErrorCode = RedisInvokeFailure;
			this->mErrorStr.assign(replay->str, replay->len);
			jsonWrite.Write("error", replay->str, replay->len);
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
		jsonWrite.Write("code", (long long)this->mErrorCode);
		jsonWrite.Write("error", this->mErrorStr.c_str(), this->mErrorStr.size());
		this->OnQueryFinish(jsonWrite);
	}
	void RedisTaskBase::AddCommandArgv(const std::string & argv)
	{
		this->mCommand.push_back(argv);
	}
	void RedisTaskBase::AddCommandArgv(const char * str, const size_t size)
	{
		this->mCommand.push_back(std::string(str, size));
	}
}