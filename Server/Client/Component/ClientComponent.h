//
// Created by leyi on 2023/9/11.
//

#ifndef APP_CLIENTCOMPONENT_H
#define APP_CLIENTCOMPONENT_H
#include"Rpc/Client/Message.h"
#include"Rpc/Client/InnerClient.h"
#include"Rpc/Interface/ISend.h"
#include"Entity/Component/Component.h"

namespace acs
{
	class ClientComponent final : public Component,
			public IRpc<rpc::Packet, rpc::Packet>, public ISender, public ILuaRegister
	{
	public:
		ClientComponent();
	public:
		int Remove(int id);
		int Connect(const std::string & address);
	private:
		bool LateAwake() final;
		void OnCloseSocket(int id, int code) final;
		int Send(int id, rpc::Packet * message) final;
		void OnLuaRegister(Lua::ModuleClass &luaRegister) final;
		void OnMessage(rpc::Packet *request, rpc::Packet *response) final;
	private:
		int OnRequest(rpc::Packet * request);
	private:
		int mIndex;
		class ProtoComponent * mProto;
		class LuaComponent * mLuaComponent;
		class DispatchComponent * mDisComponent;
		std::unordered_map<int, std::unique_ptr<rpc::InnerClient>> mClientMap;

	};
}


#endif //APP_CLIENTCOMPONENT_H
