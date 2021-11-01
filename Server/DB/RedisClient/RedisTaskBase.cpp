#include "RedisTaskBase.h"
#include <Scene/RedisComponent.h>
#include <Core/App.h>
namespace GameKeeper
{

    RedisTaskBase::RedisTaskBase(const std::string &cmd)
    {       
        this->mCommand.push_back(cmd);
        this->mStartTime = TimeHelper::GetMilTimestamp();
    }

    float RedisTaskBase::GetCostTime()
    {
        return (TimeHelper::GetMilTimestamp() - this->mStartTime) / 1000.0f;
    }

    void RedisTaskBase::DebugInvokeInfo()
    {
        std::stringstream sss;
        sss << "[" << this->GetCostTime() << "s] {";
        for (const std::string &str: this->mCommand)
        {
            sss << str << " ";
        }
        GKDebugLog(sss.str() << "}");
    }

    bool RedisTaskBase::Run()
    {
		RedisComponent * redisManager = App::Get().GetComponent<RedisComponent>();
        RedisSocket *redisSocket = redisManager->GetRedisSocket();
        if (redisSocket == nullptr)
        {
            this->mErrorStr = "redis scoket null";
            this->mErrorCode = XCode::RedisSocketIsNull;
            return true;
        }

        const char **argvArray = new const char *[this->mCommand.size()];
        size_t *argvSizeArray = new size_t[this->mCommand.size()];

        for (size_t index = 0; index < this->mCommand.size(); index++)
        {
            argvArray[index] = this->mCommand[index].c_str();
            argvSizeArray[index] = this->mCommand[index].size();
        }
        const int size = (int) this->mCommand.size();
        redisReply *replay = (redisReply *) redisCommandArgv(redisSocket, size, argvArray, argvSizeArray);

        delete[] argvArray;
        delete[] argvSizeArray;

        //redisReply * replay = (redisReply*)redisvCommand(redisSocket, this->mFormat.c_str(), this->mCommand);
        if (replay == nullptr)
        {
            this->mErrorStr = "redis replay null";
            this->mErrorCode = XCode::RedisReplyIsNull;
            return true;
        }
        switch (replay->type)
        {
            case REDIS_REPLY_STATUS:
                this->mErrorCode = XCode::Successful;
                this->mQueryDatas.push_back(std::string(replay->str, replay->len));
                break;
            case REDIS_REPLY_ERROR:
                this->mErrorCode = RedisInvokeFailure;
                this->mErrorStr.assign(replay->str, replay->len);
                break;
            case REDIS_REPLY_INTEGER:
                this->mErrorCode = XCode::Successful;
                this->mQueryDatas.push_back(std::to_string(replay->integer));
                break;
            case REDIS_REPLY_NIL:
                this->mErrorCode = XCode::Successful;
                break;
            case REDIS_REPLY_STRING:
                this->mErrorCode = XCode::Successful;
                this->mQueryDatas.push_back(std::string (replay->str, replay->len));
                break;
            case REDIS_REPLY_ARRAY:
                this->mErrorCode = XCode::Successful;
                for (size_t index = 0; index < replay->elements; index++)
                {
                    redisReply *redisData = replay->element[index];
                    if(redisData->type == REDIS_REPLY_INTEGER)
                    {
                        this->mQueryDatas.push_back(std::to_string(redisData->integer));
                    }
                    else if(redisData->type == REDIS_REPLY_STRING)
                    {
                        this->mQueryDatas.push_back(std::string(redisData->str, redisData->len));
                    }
                }
                break;
        }
        freeReplyObject(replay);
        return true;
    }

    void RedisTaskBase::AddCommandArgv(const std::string &argv)
    {
        this->mCommand.push_back(argv);
    }

    void RedisTaskBase::AddCommandArgv(const char *str, const size_t size)
    {
        this->mCommand.push_back(std::string(str, size));
    }

    bool RedisTaskBase::GetOnceData(std::string &value)
    {
        if (this->mErrorCode != XCode::Successful)
        {
            return false;
        }
        if (this->mQueryDatas.empty())
        {
            return false;
        }
        value = this->mQueryDatas[0];
        return true;
    }
}// namespace GameKeeper