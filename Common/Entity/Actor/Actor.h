//
// Created by yjz on 2023/5/17.
//

#ifndef _ACTOR_H_
#define _ACTOR_H_
#include"Entity/Unit/Unit.h"
#include"Rpc/Client/Message.h"
#include"Proto/Include/Message.h"
namespace Tendo
{
	class Actor : public Unit
	{
	 public:
		Actor(long long id, std::string  addr);
	 public:
		bool LateAwake() override;
		int Send(const std::string& func);
		int Send(const std::shared_ptr<Msg::Packet> & message);
		int Send(const std::string& func, const pb::Message& request);
	 public:
		int Call(const std::string & func);
		int Call(const std::string & func, const pb::Message & request);
		int Call(const std::string & func, std::shared_ptr<pb::Message> response);
		int Call(const std::string & func, const pb::Message & request, std::shared_ptr<pb::Message> response);
	 public:
		const std::string& GetName() { return this->mName; }
		const std::string& GetActorAddr() { return this->mAddr; }
		void SetName(const std::string& name) { this->mName = name; }
	protected:
		virtual int GetAddress(const std::string & func, std::string & addr);
		std::shared_ptr<Msg::Packet> Make(const std::string & func, const pb::Message * message);
	 private:
		std::string mAddr;
		std::string mName;
	protected:
		class InnerNetComponent * mNetComponent;
	};
}

#endif //_ACTOR_H_
