//
// Created by yjz on 2022/4/16.
//

#ifndef _DATAMGRCOMPONENT_H_
#define _DATAMGRCOMPONENT_H_
#include"Component/Component.h"
#include"Coroutine/CoroutineLock.h"
namespace Sentry
{
    class MongoDataComponent final : public Component
	{
    public:
		XCode Set(long long key, std::shared_ptr<Message> message, bool insert = true);
        XCode Set(const std::string & key, std::shared_ptr<Message> message, bool insert = true);
    public:
		std::shared_ptr<Message> Get(long long key, const std::string & tab);
        std::shared_ptr<Message> Get(const std::string & key, const std::string & tab);
    private:
		bool LateAwake() final;
    private:
        class ProtoComponent * mProtoComponent;
        class RedisDataComponent * mRedisComponent;
        class MongoAgentComponent * mMongoComponent;
    };
}

#endif //_DATAMGRCOMPONENT_H_
