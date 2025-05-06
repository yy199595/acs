//
// Created by 64658 on 2025/4/11.
//

#ifndef APP_LOCALNETCOMPONENT_H
#define APP_LOCALNETCOMPONENT_H
#include "Entity/Component/Component.h"
#include "Server/Component/ITcpComponent.h"

typedef asio::local::stream_protocol::socket LocalSocket;
typedef asio::local::stream_protocol::endpoint LocalEndpoint;
typedef asio::local::stream_protocol::acceptor LocalAcceptor;

namespace acs
{
	class LocalNetComponent : public Component, public INetListen
	{
	public:
		LocalNetComponent();
	private:
		bool LateAwake() final;
		bool StopListen() final;
		bool StartListen(const acs::ListenConfig &listen) final;
	private:
		void StartAcceptor();
		void OnAcceptor(LocalSocket * localSocket);
	private:
		Asio::Executor mExecutor;
		class ThreadComponent * mThread;
		std::unique_ptr<LocalAcceptor> mAcceptor;
	};
}


#endif //APP_LOCALNETCOMPONENT_H
