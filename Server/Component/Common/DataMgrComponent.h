//
// Created by yjz on 2022/4/16.
//

#ifndef _DATAMGRCOMPONENT_H_
#define _DATAMGRCOMPONENT_H_
#include"Pool/DataPool.h"
#include"Component/Component.h"
namespace Sentry
{
    class DataMgrComponent final : public Component
	{
    public:
		XCode Set(long long key, std::shared_ptr<Message> message);
        XCode Set(const std::string & key, std::shared_ptr<Message> message);
    public:
		std::shared_ptr<Message> Get(long long key, const std::string & tab);
        std::shared_ptr<Message> Get(const std::string & key, const std::string & tab);
    private:
		bool LateAwake() final;
    private:
        class ProtoComponent * mProtoComponent;
        class RedisDataComponent * mRedisComponent;
        class MongoAgentComponent * mMongoComponent;
        typedef Pool::DataPool<long long, Message> NumberMessagePool;
        typedef Pool::DataPool<std::string, Message> StringMessagePool;
    };
}

#endif //_DATAMGRCOMPONENT_H_
