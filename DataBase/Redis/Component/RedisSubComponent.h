//
// Created by zmhy0073 on 2022/8/3.
//

#ifndef APP_REDISSUBCOMPONENT_H
#define APP_REDISSUBCOMPONENT_H
#include"RedisComponent.h"
namespace Sentry
{
    class RedisSubComponent : public RedisComponent
    {
    public:
        RedisSubComponent() = default;
    public:
        bool IsRunCommand() final { return false; }
        bool SubscribeChannel(const std::string & name, const std::string& channel);
        long long Publish(const std::string & name, const std::string& channel, const std::string& message);
    private:
        bool Invoke(const std::string & channel, const std::string & message);
    protected:
        void OnNotFindResponse(long long taskId, std::shared_ptr<RedisResponse> message) final;
    private:
        RedisClientConfig mConfig;
        std::string mData;
        std::string mLocalHost;
        class LuaScriptComponent * mLuaComponent;
    };
}


#endif //APP_REDISSUBCOMPONENT_H
