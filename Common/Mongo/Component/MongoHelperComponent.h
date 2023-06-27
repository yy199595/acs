//
// Created by yjz on 2022/8/28.
//

#ifndef _MONGOAGENTCOMPONENT_H_
#define _MONGOAGENTCOMPONENT_H_
#include "Message/s2s/db.pb.h"
#include"Entity/Component/Component.h"


using namespace google::protobuf;
namespace Tendo
{
	class MongoHelperComponent final : public Component
	{
	 public:
		MongoHelperComponent() = default;
		~MongoHelperComponent() final = default;
	 public:
		int Insert(const Message & message);
		int Insert(const char * tab, const Message & message);
		int Insert(const char * tab, const std::string & json);
		int Remove(const char * tab, const std::string & select, int limit);
		int Update(const char * tab, const std::string & select, const std::string & data);
		int Query(const char * tab, const std::string & select, std::shared_ptr<Message> & response);
    public:
		bool Awake() final;
		int Save(const Message & message);
		int Save(const char * tab, long long id, const std::string & data);
		int Save(const char * tab, const std::string & id, const std::string & data);
	 private:
		std::string mRpc;
		std::string mServer;
	};
}

#endif //_MONGOAGENTCOMPONENT_H_
