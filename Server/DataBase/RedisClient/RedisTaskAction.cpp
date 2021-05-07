#include "RedisTaskAction.h"
#include<Manager/RedisManager.h>
namespace DataBase
{
	RedisTaskAction::RedisTaskAction(RedisManager * mgr, long long id, const std::string & command)
		:ThreadTaskAction(mgr, id)
	{
		mCommand = command;
		this->mRedisManager = mgr;
	}
	void RedisTaskAction::InvokeInThreadPool(long long threadId)
	{
		RedisSocket * redisSocket = this->mRedisManager->GetRedisSocket(threadId);
		if (redisSocket == nullptr)
		{
			return;
		}
		PB::NetWorkPacket data;
		data.set_error_code(XCode::CallFunctionNotExist);
		data.set_func_name("RedisManager.Query");
		data.set_callback_id(456465465454);
		data.set_operator_id(199595959595);

		std::string name = data.SerializeAsString();
		redisReply * replay = (redisReply*)redisCommand(redisSocket, "%s %s %b", "SET", "name1", name.c_str(), name.size());
		
		redisReply * replay1 = (redisReply*)redisCommand(redisSocket, "%s %s", "GET", "name1");

		std::string str(replay1->str, replay1->len);


		switch (replay->type)
		{
		case REDIS_REPLY_STATUS:
			SayNoDebugInfo(std::string(replay->str, replay->len));
			break;
		case REDIS_REPLY_ERROR:
			SayNoDebugError(std::string(replay->str, replay->len));
			break;
		case REDIS_REPLY_INTEGER:
			break;
		case REDIS_REPLY_NIL:
			break;
		case REDIS_REPLY_STRING:		
			break;
		case REDIS_REPLY_ARRAY:
			break;
		}
	}
}