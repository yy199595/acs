//
// Created by leyi on 2023/9/11.
//

#ifndef APP_WSCLIENTCOMPONENT_H
#define APP_WSCLIENTCOMPONENT_H
#include"Rpc/Common/Message.h"
#include"Entity/Component/Component.h"
#include"WebSocket/Client/WebSocketClient.h"

namespace acs
{
	class WsClientComponent final : public Component,
									 public IRpc<rpc::Message, rpc::Message>, public rpc::IInnerSender, public ILuaRegister
	{
	public:
		WsClientComponent();
	public:
		int Remove(int id);
		int Connect(const std::string & address);
	private:
		bool LateAwake() final;
		void OnClientError(int id, int code) final;
		void OnSendFailure(int id, rpc::Message *message) final;
		void OnLuaRegister(Lua::ModuleClass &luaRegister) final;
		void OnMessage(rpc::Message *request, rpc::Message *response) final;
	private:
		int Send(int id, rpc::Message * message) final;
		char GetNet() const final { return rpc::Net::Client; }
	private:
		int OnRequest(rpc::Message * request);
	private:
		int mIndex;
		class ProtoComponent * mProto;
		class LuaComponent * mLuaComponent;
		class DispatchComponent * mDisComponent;
		std::queue<std::unique_ptr<ws::RequestClient>> mClientQueue;
		std::unordered_map<int, std::shared_ptr<ws::RequestClient>> mClientMap;

	};
}


#endif //APP_WSCLIENTCOMPONENT_H
