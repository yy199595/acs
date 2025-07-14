//
// Created by leyi on 2023/6/7.
//

#ifndef APP_ROUTERCOMPONENT_H
#define APP_ROUTERCOMPONENT_H
#include <queue>
#include "Rpc/Common/Message.h"
#include "Entity/Component/Component.h"
struct lua_State;
namespace acs
{
	class Node;
	class RouterComponent final : public Component, public ISystemUpdate, public IServerRecord
	{
	public:
		RouterComponent();
	public:
		rpc::IInnerSender * GetSender(char net);
		int Send(int id, std::unique_ptr<rpc::Message> & message);
		int Send(int id, int code, std::unique_ptr<rpc::Message> & message);
		int LuaCall(lua_State * lua, int id, std::unique_ptr<rpc::Message> & message);
		std::unique_ptr<rpc::Message> Call(int id, std::unique_ptr<rpc::Message> & message);
	private:
		bool Awake() final;
		bool LateAwake() final;
		void OnSystemUpdate() noexcept final;
		void OnRecord(json::w::Document &document) final;
	private:
		class DispatchComponent * mDispatch;
		std::unordered_map<char, rpc::IInnerSender *> mSenders;
		std::queue<std::unique_ptr<rpc::Message>> mLocalMessages;
	};
}


#endif //APP_ROUTERCOMPONENT_H
