//
// Created by yjz on 2023/5/17.
//

#ifndef _ACTOR_H_
#define _ACTOR_H_
#include"Entity/Unit/Unit.h"
#include<google/protobuf/message.h>
namespace pb = google::protobuf;
namespace Tendo
{
	class Actor : public Unit
	{
	 public:
		Actor(long long id, const std::string& addr);
	 public:
		bool LateAwake() final;
		int Send(const std::string& func);
		int Send(const std::string& func, const pb::Message& request);
	 public:
		int Call(const std::string & func);
		int Call(const std::string & func, const pb::Message & request);
		int Call(const std::string & func, std::shared_ptr<pb::Message> response);
		int Call(const std::string & func, const pb::Message & request, std::shared_ptr<pb::Message> response);
	 public:
		const std::string& GetAddr() { return this->mAddr; }
		const std::string& GetName() { return this->mName; }
		void SetName(const std::string& name) { this->mName = name; }
	 private:
		std::string mAddr;
		std::string mName;
		class InnerNetComponent * mNetComponent;
	};
}

#endif //_ACTOR_H_
