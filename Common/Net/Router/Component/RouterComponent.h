//
// Created by leyi on 2023/6/7.
//

#ifndef APP_ROUTERCOMPONENT_H
#define APP_ROUTERCOMPONENT_H
#include<queue>
#include"Rpc/Interface/ISend.h"
#include"Entity/Component/Component.h"
struct lua_State;
namespace acs
{
	class Server;
	class RouterComponent : public Component, public ISystemUpdate, public IServerRecord
	{
	public:
		RouterComponent();
	public:
		int Send(int id, int code, rpc::Message * message);
		int Send(int id, std::unique_ptr<rpc::Message> message);
		rpc::Message * Call(int id, std::unique_ptr<rpc::Message> message);
		int LuaCall(lua_State * lua, int id, std::unique_ptr<rpc::Message> message);
	private:
		bool LateAwake() final;
		void OnSystemUpdate() noexcept final;
		void OnRecord(json::w::Document &document) final;
		ISender * GetSender(char net);
	private:
		class DispatchComponent * mDisComponent;
		std::unordered_map<char, ISender *> mSenders;
		std::queue<std::unique_ptr<rpc::Message>> mLocalMessages;
	};
}


#endif //APP_ROUTERCOMPONENT_H
