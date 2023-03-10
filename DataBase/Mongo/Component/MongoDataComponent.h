//
// Created by yjz on 2022/4/16.
//

#ifndef _DATAMGRCOMPONENT_H_
#define _DATAMGRCOMPONENT_H_
#include"Component/Component.h"
#include"Coroutine/CoroutineLock.h"
#include"google/protobuf/message.h"
using namespace google::protobuf;
namespace Sentry
{
    class MongoDataComponent final : public Component
	{
    public:
		int Set(long long key, std::shared_ptr<Message> message, bool insert = true);
        int Set(const std::string & key, std::shared_ptr<Message> message, bool insert = true);
    public:
		std::shared_ptr<Message> Get(long long key, const std::string & tab);
        std::shared_ptr<Message> Get(const std::string & key, const std::string & tab);
    private:
		bool LateAwake() final;
    private:
        class ProtoComponent * mProtoComponent;
#ifdef __ENABLE_REDIS__
        class RedisComponent * mRedisComponent;
#endif
        class MongoHelperComponent * mMongoComponent;
    };
}

#endif //_DATAMGRCOMPONENT_H_
