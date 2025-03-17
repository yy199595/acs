
#pragma once

#include"XCode/XCode.h"
#include"Message/s2s/db.pb.h"
#include"Proto/Include/Message.h"
#include"Entity/Component/Component.h"

namespace acs
{
    class MysqlComponent final : public Component
	{
	 public:
		MysqlComponent();
     public:
		int Add(const pb::Message & data, int flag);
		int Save(const pb::Message & data, int flag);
		int QueryOnce(const std::string & json, pb::Message * response);
        int Delete(const std::string & table, const std::string& deleteJson, int flag);
        int Update(const std::string & table, const std::string& updateJson, const std::string& whereJson, int flag);
    private:
		bool Awake() final;
		bool LateAwake() final;
		class Server * GetActor();
	private:
		std::string mServer;
		class ActorComponent * mActor;
	};
}