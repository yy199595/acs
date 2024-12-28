//
// Created by yjz on 2023/5/17.
//

#ifndef APP_ACTOR_H
#define APP_ACTOR_H
#include"Entity/Unit/Entity.h"
#include"Rpc/Client/Message.h"
struct lua_State;
namespace acs
{
	class Actor : public Entity
	{
	 public:
		explicit Actor(long long id, std::string name);
	 public:
		bool LateAwake() final;
		int Send(const std::string& func);
		const std::string & Name() const { return this->mName; }
		int Send(const std::string& func, const pb::Message& request);
	 public:
		int Call(const std::string & func);
		int Call(std::unique_ptr<rpc::Message> request);
		int Call(const std::string & func, pb::Message * response);
		int Call(const std::string & func, const pb::Message & request);
		int Call(const std::string & func, const pb::Message & request, pb::Message * response);
	public:
		int Publish(const std::string & event);
		int Publish(const std::string & event, json::w::Document & document);
		int Publish(const std::string & event, char proto, const std::string & data);
	public:
		int LuaSend(lua_State * lua, std::unique_ptr<rpc::Message>);
		int LuaCall(lua_State * lua, std::unique_ptr<rpc::Message>);
		int MakeMessage(lua_State * lua, int idx, const std::string & func, std::unique_ptr<rpc::Message> &) const;
	 public:
		virtual bool OnInit() = 0;
		virtual void EncodeToJson(std::string * json) = 0;
		virtual bool DecodeFromJson(const std::string & json) { return true;}
	protected:
		virtual bool GetAddress(const rpc::Message & request, int & id) const = 0;
		virtual int Make(const std::string & func, std::unique_ptr<rpc::Message> & request) const = 0;
	protected:
		class RouterComponent * mRouter;
	private:
		const std::string mName;
		class ProtoComponent * mProto;
	};
}

#endif //APP_ACTOR_H
