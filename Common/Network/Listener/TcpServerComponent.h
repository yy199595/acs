#pragma once

#include <Component/Component.h>
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
        std::string mHostIp;
		std::set<std::string> mWhiteList;    //白名单
		std::unordered_map<std::string, NetworkListener *> mListenerMap;
	};
}