//
// Created by zmhy0073 on 2022/11/2.
//

#ifndef APP_REDISSCRIPTCOMPONENT_H
#define APP_REDISSCRIPTCOMPONENT_H
#include"Redis/Client/RedisDefine.h"
#include"Entity/Component/Component.h"
namespace Tendo
{
    class RedisScriptComponent : public Component
    {
    public:
        RedisScriptComponent() = default;
    public:
        std::shared_ptr<RedisRequest> MakeLuaRequest(const std::string & fullName, const std::string & json);
        std::shared_ptr<RedisResponse> Call(const std::string & func, const std::string & json, bool async = true);
    private:
       // bool Start() final;
        bool LateAwake() final;
        bool OnLoadScript(const std::string & name, const std::string &md5);
    private:
        class RedisComponent * mComponent;
        std::unordered_map<std::string, std::string> mLuaMap;
    };
}


#endif //APP_REDISSCRIPTCOMPONENT_H
