//
// Created by 64658 on 2025/1/2.
//

#ifndef APP_WEBSOCKETCOMPONENT_H
#define APP_WEBSOCKETCOMPONENT_H
namespace ws
{
	class Message;
	class SessionClient;
	class RequestClient;
}
#include "Rpc/Interface/ISend.h"
#include "Rpc/Component/RpcComponent.h"
#include "Server/Component/ITcpComponent.h"
#include "WebSocket/Common/WebSocketMessage.h"
namespace acs
{
	class WebSocketComponent : public ITcpListen, public ISender, public IComplete,
							   public RpcComponent<ws::Message>, public IRpc<ws::Message, ws::Message>
	{
	public:
		WebSocketComponent();
	public:
		void Complete() final;
		bool LateAwake() final;
		bool OnListen(tcp::Socket *socket) final;
		void OnClientError(int id, int code) final;
		void OnMessage(int, ws::Message *request, ws::Message *response) final;
	public:
		int Send(int id, rpc::Message *message) final;
	private:
		class ActorComponent * mActor;
		class ThreadComponent * mThread;
		math::NumberPool<int> mClientPool;
		class DispatchComponent * mDispatch;
		std::unordered_map<int, std::shared_ptr<ws::RequestClient>> mClients;
		std::unordered_map<int, std::shared_ptr<ws::SessionClient>> mSessions;
	};
}


#endif //APP_WEBSOCKETCOMPONENT_H
