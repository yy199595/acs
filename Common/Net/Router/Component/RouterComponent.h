//
// Created by leyi on 2023/6/7.
//

#ifndef APP_ROUTERCOMPONENT_H
#define APP_ROUTERCOMPONENT_H
#include"Rpc/Interface/ISend.h"
#include"Entity/Component/Component.h"
struct lua_State;
namespace joke
{
	class Server;
	class RouterComponent : public Component
	{
	public:
		RouterComponent();
	public:
		int Send(int id, int code, rpc::Packet * message);
		int Send(int id, std::unique_ptr<rpc::Packet> message);
		rpc::Packet * Call(int id, std::unique_ptr<rpc::Packet> message);
		int LuaCall(lua_State * lua, int id, std::unique_ptr<rpc::Packet> message);
	private:
		bool LateAwake() final;
		ISender * GetSender(unsigned int net);
	private:
		class DispatchComponent * mDisComponent;
		std::unordered_map<unsigned int, ISender *> mSenders;
	};
}


#endif //APP_ROUTERCOMPONENT_H
