//
// Created by yjz on 2022/8/28.
//

#ifndef _MONGOSYNCCOMPONENT_H_
#define _MONGOSYNCCOMPONENT_H_
#include"Message/db.pb.h"
#include"Component/Component.h"
namespace Sentry
{
	class DataSyncComponent : public Component
	{
	 public:
		DataSyncComponent() = default;
		~DataSyncComponent() = default;
	 public:
        void Del(const std::string & id, const std::string & fullName);
        void Del(const std::string & id, const std::string & db, const std::string & tab);
        void Set(const std::string & id, const std::string & fullName, const std::string & json);
        void Set(const std::string & id, const std::string & db, const std::string & tab, const std::string & json);
    private:
		bool LateAwake() final;
	 private:
		class RedisDataComponent * mRedisComponent;
	};
}

#endif //_MONGOSYNCCOMPONENT_H_
