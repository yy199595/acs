//
// Created by yjz on 2022/8/28.
//

#ifndef _MONGOSYNCCOMPONENT_H_
#define _MONGOSYNCCOMPONENT_H_
#include"Message/s2s.pb.h"
#include"Component/Component.h"
namespace Sentry
{
	class DataSyncRedisComponent : public Component
	{
	 public:
		DataSyncRedisComponent() = default;
		~DataSyncRedisComponent() = default;
	 public:
        void Del(const std::string & id, const std::string & tab);
        void Set(const std::string & id, const s2s::mongo::insert & data);
		void Set(const std::string & id, const std::string & tab, const std::string & json);
    private:
		bool LateAwake() final;
	 private:
		class RedisDataComponent * mRedisComponent;
	};
}

#endif //_MONGOSYNCCOMPONENT_H_
