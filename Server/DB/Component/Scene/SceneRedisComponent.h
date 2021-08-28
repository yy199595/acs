#pragma once

#include<Util/NumberHelper.h>
#include <Component/Component.h>
#include<RedisClient/RedisTask.h>
#include <Coroutine/CoroutineComponent.h>
#include<QueryResult/InvokeResultData.h>
using namespace Sentry;
namespace Sentry
{
    class SceneTaskComponent;

    class SceneRedisComponent : public Component
    {
    public:
        SceneRedisComponent();

        ~SceneRedisComponent() {}

		bool CloseRedisSocket();
        RedisSocket *GetRedisSocket();

    protected:
        bool Awake() final;            //初始化管理器
        void Start() override;                //初始化完成之后
    public:
        template<typename ... Args>
		RedisTask * CreateTask(const std::string &cmd, Args &&... args);
		redisContext * ConnectRedis(int timeout = 3);
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
		SceneTaskComponent *mTaskManager;
        CoroutineComponent *mCorComponent;
        std::unordered_map<std::thread::id, redisContext *> mRedisContextMap;
    };

    template<typename ...Args>
    inline RedisTask * SceneRedisComponent::CreateTask(const std::string &cmd, Args &&...args)
    {
        if (sizeof ...(Args) == 0)
        {
            return nullptr;
        }
        if (this->mCorComponent->IsInMainCoroutine())
        {
            SayNoDebugError("[redis error] redis not in coreoutine");
            return nullptr;
        }

		RedisTask * taskAction = new RedisTask(cmd);
        if (taskAction != nullptr)
        {
            taskAction->InitCommand(std::forward<Args>(args)...);
            return taskAction;
        }
        return nullptr;
    }
}