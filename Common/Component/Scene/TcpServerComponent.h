#pragma once

#include <Component/Component.h>

namespace Sentry
{
	struct NetListenerInfo
	{
	public:
		std::string Name;
		unsigned int Count;
		unsigned short Port;
		std::string ListenIp;
		AsioWork * TcptWork;
		AsioContext * TcpContext;
		std::string HandlerComponent;
		class NetWorkThread * NetThread;
		class NetworkListener * TcpListener;
	};
}

namespace Sentry
{
	// 处理外部连接进来的session
	class NetworkListener;
	class TcpNetSessionComponent;
	class TcpServerComponent : public Component
	{
	public:
		TcpServerComponent() {}

		~TcpServerComponent() {}

	protected:
		bool Awake() override;
		void Start() override;		
	private:		
		std::set<std::string> mWhiteList;    //白名单
		std::unordered_map<std::string, NetworkListener *> mListenerMap;
	};
}