//
// Created by zmhy0073 on 2022/8/3.
//

#ifndef APP_REDISSUBCOMPONENT_H
#define APP_REDISSUBCOMPONENT_H
#include"RedisComponent.h"
namespace Sentry
{
    class RedisSubComponent : public RedisComponent, public IStart
    {
    public:
        RedisSubComponent() = default;
    public:
        bool SubscribeChannel(const std::string& channel);
        long long Publish(const std::string& channel, const std::string& message);
    protected:
        bool OnStart() final;
        bool LateAwake() final;
        void OnNotFindResponse(long long taskId, std::shared_ptr<RedisResponse> message) final;
    private:
        RedisConfig mConfig;
        std::string mData;
        std::string mAddress;
        SharedRedisClient mSubClient;
        class LuaScriptComponent * mLuaComponent;
    };
}


#endif //APP_REDISSUBCOMPONENT_H
