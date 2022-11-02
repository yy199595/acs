//
// Created by zmhy0073 on 2022/11/2.
//

#ifndef APP_REDISSCRIPTCOMPONENT_H
#define APP_REDISSCRIPTCOMPONENT_H
#include"Client/RedisDefine.h"
#include"Component/Component.h"
namespace Sentry
{
    class RedisScriptComponent : public Component, public IStart
    {
    public:
        RedisScriptComponent() = default;
    public:
        std::shared_ptr<RedisRequest> MakeLuaRequest(const std::string & fullName, const std::string & json);
        std::unique_ptr<std::string> Call(const std::string & name, const std::string & func, const std::string & json);
    private:
        bool Start() final;
        bool LateAwake() final;
        void OnLoadScript(const std::string &path, const std::string &md5);
    private:
        class RedisComponent * mComponent;
        std::unordered_map<std::string, std::string> mLuaMap;
    };
}


#endif //APP_REDISSCRIPTCOMPONENT_H
