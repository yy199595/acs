//
// Created by yjz on 2022/4/16.
//

#ifndef _DATAMGRCOMPONENT_H_
#define _DATAMGRCOMPONENT_H_
#include"Component/Component.h"
namespace Sentry
{
	class DataMgrComponent final : public Component, public ISecondUpdate
	{
	 public:
		XCode Set(const std::string & tab, long long key, const Message & data);
		XCode Set(const std::string & tab, const std::string & key, const Message & data);
	 public:
		XCode Get(const std::string & tab, long long key, std::shared_ptr<Message> result);
		XCode Get(const std::string & tab, const std::string & key, std::shared_ptr<Message> result);
	 protected:
		void OnSecondUpdate() final;
	 protected:
		bool LateAwake() final;
	 private:
		class RedisComponent * mRedisComponent;
		class MysqlProxyComponent * mMysqlComponent;
	};
}

#endif //_DATAMGRCOMPONENT_H_
