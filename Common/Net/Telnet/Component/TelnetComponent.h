//
// Created by 64658 on 2025/3/4.
//

#ifndef APP_TELNETCOMPONENT_H
#define APP_TELNETCOMPONENT_H
#include "Proto/Message/IProto.h"
#include "Telnet/Client/TelnetClient.h"
#include "Entity/Component/Component.h"
#include "Server/Component/ITcpComponent.h"
#include "Lua/Module/LuaModule.h"

namespace acs
{
	class TelnetComponent : public Component, public IRpc<telnet::Request, telnet::Response>, public ITcpListen
	{
	public:
		TelnetComponent();
	private:
		bool LateAwake() final;
		bool Send(int id, const std::string & msg);
		bool Send(int id, std::unique_ptr<telnet::Response> response);
		void OnClientError(int id, int code) final;
		bool OnListen(tcp::Socket *socket) noexcept final;
		void OnMessage(int id, telnet::Request *request, telnet::Response *response) noexcept final;
	private:
		class RouterComponent * mRouter;
		math::NumberPool<int> mNumberPool;
		std::unordered_map<int, std::shared_ptr<telnet::Client>> mClients;
	};
}


#endif //APP_TELNETCOMPONENT_H
