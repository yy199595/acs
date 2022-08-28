//
// Created by yjz on 2022/8/28.
//

#ifndef _MONGOSYNCCOMPONENT_H_
#define _MONGOSYNCCOMPONENT_H_
#include"Message/s2s.pb.h"
#include"Component/Component.h"
namespace Sentry
{
	class MongoSyncComponent : public Component
	{
	 public:
		MongoSyncComponent() = default;
		~MongoSyncComponent() = default;
	 public:
		void OnInsert(const s2s::mongo::index & data);
		void OnQuery(long long _id, const std::string & tab, const std::string & json);
		void OnQuery(const std::string & _id, const std::string & tab, const std::string & json);
	 private:
		bool LateAwake() final;
	 private:
		class RedisDataComponent * mRedisComponent;
	};
}

#endif //_MONGOSYNCCOMPONENT_H_
