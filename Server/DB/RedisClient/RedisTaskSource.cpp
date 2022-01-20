#include "RedisTaskSource.h"
#include "Component/RedisComponent.h"
#include <Core/App.h>
#include"Scene/ThreadPoolComponent.h"
#ifdef REDIS_SAVE_JSON
#include<google/protobuf/util/json_util.h>
#endif
namespace Sentry
{

    RedisTaskSource::RedisTaskSource(const std::string &cmd)
    {
        this->mCommand.emplace_back(cmd);
        this->mStartTime = Helper::Time::GetMilTimestamp();
        this->mRedisComponent = App::Get().GetComponent<RedisComponent>();
    }

    RedisTaskSource::~RedisTaskSource()
    {
        this->mCommand.clear();
    }

    std::shared_ptr<RedisResponse> RedisTaskSource::Await()
    {
        auto threadComponent =
                App::Get().GetComponent<ThreadPoolComponent>();
        threadComponent->StartTask(this);
        return this->mTaskSource.Await();
    }

    bool RedisTaskSource::Run()
    {
        RedisSocket *redisSocket = nullptr;
        if (redisSocket == nullptr)
        {
            const std::string &err = "redis socket null";
            this->mTaskSource.SetResult(std::make_shared<RedisResponse>(XCode::RedisSocketIsNull, err));
            return true;
        }
        size_t size = (int) this->mCommand.size();
        std::unique_ptr<size_t[]> sizeArray(new size_t[size]);
        std::unique_ptr<const char *[]> dataArray(new const char *[size]);

#ifdef __DEBUG__ && __REDIS_DEBUG_LOG__
        //std::cout << "redis cmd : ";
#endif
        for (size_t index = 0; index < this->mCommand.size(); index++)
        {
            sizeArray[index] = this->mCommand[index].size();
            dataArray[index] = this->mCommand[index].c_str();
#ifdef __DEBUG__ && __REDIS_DEBUG_LOG__
            //std::cout << this->mCommand[index] << " ";
#endif
        }
#ifdef __DEBUG__ && __REDIS_DEBUG_LOG__
        //std::cout << std::endl;
#endif

        auto reply = (redisReply *) redisCommandArgv(
                redisSocket, (int) size, dataArray.get(), sizeArray.get());

        if (reply == nullptr)
        {
            const std::string err = "redis replay null";
            this->mTaskSource.SetResult(std::make_shared<RedisResponse>(XCode::RedisReplyIsNull, err));
            return true;
        }
        if (reply->type == REDIS_REPLY_ERROR)
        {
            const std::string err(reply->str, reply->len);
            this->mTaskSource.SetResult(std::make_shared<RedisResponse>(XCode::RedisInvokeFailure, err));
            return true;
        }
        this->mTaskSource.SetResult(std::make_shared<RedisResponse>(reply));
        return true;
    }

    void RedisTaskSource::AddCommandArgv(const std::string &argv)
    {
        this->mCommand.emplace_back(argv);
    }

    void RedisTaskSource::AddCommandArgv(const char *str, const size_t size)
    {
        this->mCommand.emplace_back(str, size);
    }

    void RedisTaskSource::AddCommand(const Message &value)
    {
#ifdef REDIS_SAVE_JSON
        std::string json;
        util::MessageToJsonString(value, &json);
        this->mCommand.emplace_back(json);
#else
        std::string message;
        if(value.SerializeToString(&message))
        {
            this->mCommand.emplace_back(message);
        }
#endif
    }

    void RedisTaskSource::AddCommand(const std::vector<std::string> &commands)
    {
        for(const std::string & cmd : commands)
        {
            this->mCommand.emplace_back(std::move(cmd));
        }
    }

}// namespace Sentry