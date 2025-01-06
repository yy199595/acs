//
// Created by leyi on 2023/9/11.
//

#ifndef APP_TCPCLIENTCOMPONENT_H
#define APP_TCPCLIENTCOMPONENT_H
#include"Rpc/Client/Message.h"
#include"Rpc/Client/InnerClient.h"
#include"Entity/Component/Component.h"

namespace acs
{
	class TcpClientComponent final : public Component,
									 public IRpc<rpc::Message, rpc::Message>, public rpc::IInnerSender, public ILuaRegister
	{
	public:
		TcpClientComponent();
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
		std::queue<std::unique_ptr<rpc::InnerClient>> mClientQueue;
		std::unordered_map<int, std::shared_ptr<rpc::InnerClient>> mClientMap;

	};
}


#endif //APP_TCPCLIENTCOMPONENT_H
