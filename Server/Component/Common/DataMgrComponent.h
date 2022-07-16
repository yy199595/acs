//
// Created by yjz on 2022/4/16.
//

#ifndef _DATAMGRCOMPONENT_H_
#define _DATAMGRCOMPONENT_H_
#include"Component/Component.h"
namespace Sentry
{
	class DataMgrComponent final : public Component
	{
	 public:
		XCode Set(long long key, const Message & message);
		XCode Set(const std::string & key, const Message & message);
	 public:
		XCode Get(long long key, std::shared_ptr<Message> result);
		XCode Get(const std::string & key, std::shared_ptr<Message> result);
	 public:
		XCode Add(long long key, const Message & message);
		XCode Add(const std::string & key, const Message & message);
	 protected:
		bool LateAwake() final;
	 private:
		class MainRedisComponent * mRedisComponent;
		class MysqlDataComponent * mMysqlComponent;
	};
}

#endif //_DATAMGRCOMPONENT_H_
