#pragma once

#include "RedisDefine.h"
#include<Thread/TaskProxy.h>
#include<XCode/XCode.h>
#include<memory>
#include<string>
#include<queue>
#include<google/protobuf/message.h>
namespace GameKeeper
{
    class RedisComponent;

    class QuertJsonWritre;

    class RedisTaskBase : public TaskProxy
    {
    public:
        explicit RedisTaskBase(const std::string &cmd);
        ~RedisTaskBase() override;
    public:
        bool Run() final;//在线程池执行的任务
    public:
        template<typename... Args>
        void InitCommand(Args &&...args);      
    private:
        //inline void AddCommand(const char *value) { this->mCommand.push_back(value); }

        inline void AddCommand(const std::string &value) { this->mCommand.emplace_back(value); }

        inline void AddCommand(const int value) { this->mCommand.emplace_back(std::to_string(value)); }

        inline void AddCommand(const float value) { this->mCommand.emplace_back(std::to_string(value)); }

        inline void AddCommand(const double value) { this->mCommand.emplace_back(std::to_string(value)); }

        inline void AddCommand(const long long value) { this->mCommand.emplace_back(std::to_string(value)); }

        void AddCommand(const google::protobuf::Message & value);
    private:
        void Encode() {}

        template<typename T, typename... Args>
        inline void Encode(const T &t, Args... args)
        {
            this->AddCommand(t);
            this->Encode(std::forward<Args>(args)...);
        }

    public:
        void AddCommandArgv(const std::string &argv);

        void AddCommandArgv(const char *str, size_t size);

        std::shared_ptr<RedisResponse> GetResponse() { return this->mResponse;}
	private:
		long long mStartTime;
        RedisComponent * mRedisComponent;
		std::vector<std::string> mCommand;
        std::shared_ptr<RedisResponse> mResponse;
    };

    template<typename... Args>
    inline void RedisTaskBase::InitCommand(Args &&...args)
    {
        this->Encode(std::forward<Args>(args)...);
    }
}// namespace GameKeeper