//
// Created by leyi on 2023/6/7.
//

#ifndef APP_ROUTERCOMPONENT_H
#define APP_ROUTERCOMPONENT_H
#include"Rpc/Interface/ISend.h"
#include"Entity/Component/Component.h"
struct lua_State;
namespace Tendo
{
	class ServerActor;
	class RouterComponent : public Component
	{
	public:
		RouterComponent();
	private:
		bool LateAwake() final;
	public:
		int Send(const std::string & addr, const std::shared_ptr<Msg::Packet> & message);
		int Send(const std::string & addr, int code, const std::shared_ptr<Msg::Packet> & message);
		std::shared_ptr<Msg::Packet> Call(const std::string &addr, const std::shared_ptr<Msg::Packet> & message);
	public:
		int LuaCall(lua_State * lua, const std::string & address, const std::shared_ptr<Msg::Packet> & message);
	private:
		ISender * GetSender(int net);
	private:
		class DispatchComponent * mDisComponent;
		std::unordered_map<int, ISender *> mSenders;
	};
}


#endif //APP_ROUTERCOMPONENT_H
