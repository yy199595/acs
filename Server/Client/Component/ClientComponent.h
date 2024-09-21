//
// Created by leyi on 2023/9/11.
//

#ifndef APP_CLIENTCOMPONENT_H
#define APP_CLIENTCOMPONENT_H
#include"Rpc/Client/Message.h"
#include"Core/Map/HashMap.h"
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
		int Connect(const std::string & address);
		int Send(int id, rpc::Packet * message) final;
	private:
		bool LateAwake() final;
		void OnLuaRegister(Lua::ModuleClass &luaRegister) final;
		void OnMessage(rpc::Packet *request, rpc::Packet *response) final;
	private:
		int OnRequest(rpc::Packet * request);
	private:
		class ProtoComponent * mProto;
		class LuaComponent * mLuaComponent;
		class DispatchComponent * mDisComponent;
		custom::HashMap<int, rpc::InnerClient *> mClientMap;

	};
}


#endif //APP_CLIENTCOMPONENT_H
