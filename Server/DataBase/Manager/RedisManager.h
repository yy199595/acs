#pragma once

#include<Util/NumberHelper.h>

#include<Coroutine/CoroutineManager.h>
#include<QueryResult/InvokeResultData.h>
#include<RedisClient/RedisTaskAction.h>

using namespace Sentry;
namespace Sentry
{
    class ThreadTaskManager;

    class RedisManager : public Manager
    {
    public:
        RedisManager();

        ~RedisManager() {}

        RedisSocket *GetRedisSocket(long long id);

    protected:
        bool OnInit() final;            //初始化管理器
        void OnInitComplete() override;                //初始化完成之后
    public:
        template<typename ... Args>
        RedisSharedTask CreateTask(const std::string &cmd, Args &&... args);

    public:
        bool HasValue(const std::string &key);

        bool HasValue(const std::string &tab, const std::string &key);

    public:
        bool DelValue(const std::string &key);

        bool DelValue(const std::string &tab, const std::string &key);

    public:
        bool SetValue(const std::string &key, const std::string &value);

        bool SetValue(const std::string &key, const std::string &value, int second);

        bool SetValue(const std::string &tab, const std::string &key, const std::string &value);

        bool SetValue(const std::string &tab, const std::string &key, const shared_ptr<Message> value);

    public:
        bool GetValue(const std::string &key, std::string &value);

        bool GetValue(const std::string &tab, const std::string &key, std::string &value);

        bool GetValue(const std::string &tab, const std::string &key, shared_ptr<Message> value);

    private:
        std::string mRedisIp;        //redis ip地址
        unsigned short mRedisPort;    //端口号
        CoroutineManager *mCorManager;;
        ThreadTaskManager *mTaskManager;


        std::unordered_map<long long, redisContext *> mRedisContextMap;
    };

    template<typename ...Args>
    inline RedisSharedTask RedisManager::CreateTask(const std::string &cmd, Args &&...args)
    {
        if (sizeof ...(Args) == 0)
        {
            return nullptr;
        }
        if (this->mCorManager->IsInMainCoroutine())
        {
            SayNoDebugError("[redis error] redis not in coreoutine");
            return nullptr;
        }

        RedisSharedTask taskAction = make_shared<RedisTaskAction>(this, cmd);
        if (taskAction != nullptr)
        {
            taskAction->InitCommand(std::forward<Args>(args)...);
            return taskAction;
        }
        return nullptr;
    }
}