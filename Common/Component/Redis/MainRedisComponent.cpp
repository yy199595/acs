#include"MainRedisComponent.h"

#include"App/App.h"
#include"Util/DirectoryHelper.h"
#include"Script/ClassProxyHelper.h"
#include"Global/ServiceConfig.h"
#include"DB/Redis/RedisClientContext.h"
#include"Script/Extension/Redis/LuaRedis.h"
#include"Component/Scene/NetThreadComponent.h"

namespace Sentry
{
	bool MainRedisComponent::LateAwake()
	{
        RedisComponent::LateAwake();
		this->mTaskComponent = this->GetComponent<TaskComponent>();
		LOG_CHECK_RET_FALSE(this->GetComponent<NetThreadComponent>());
		return true;
	}

	bool MainRedisComponent::OnStart()
	{
        const rapidjson::Value* jsonValue = this->GetApp()->GetConfig().GetJsonValue("redis");
        if (jsonValue == nullptr || !jsonValue->IsObject())
        {
            return false;
        }
        auto iter = jsonValue->MemberBegin();
        for (; iter != jsonValue->MemberEnd(); iter++)
        {
            const char * name = iter->name.GetString();
            if(this->ParseConfig(name, iter->value) == nullptr)
            {
                LOG_ERROR("check " << name << " redis config error");
                return false;
            }
            if(this->RunCmd(name, "PING")->HasError())
            {
                return false;
            }
        }
        return true;
	}

    void MainRedisComponent::OnSecondUpdate(const int tick)
    {

    }


    bool MainRedisComponent::Call(const std::string &name, const std::string &func, Json::Writer &request,
                                  std::shared_ptr<Json::Reader> response)
    {
        SharedRedisClient redisClientContext = this->GetClient(name);
        std::shared_ptr<RedisRequest> redisRequets = this->MakeLuaRequest(func, request.JsonString());
        std::shared_ptr<RedisResponse> redisResponse = this->Run(redisClientContext, redisRequets);
        if (redisResponse == nullptr || redisResponse->HasError())
        {
            return false;
        }
        return response->ParseJson(redisResponse->GetString());
    }

	void MainRedisComponent::OnLuaRegister(Lua::ClassProxyHelper& luaRegister)
	{
		luaRegister.BeginRegister<MainRedisComponent>();
		luaRegister.PushExtensionFunction("Run", Lua::Redis::Run);
		luaRegister.PushExtensionFunction("Call", Lua::Redis::Call);
	}
}
