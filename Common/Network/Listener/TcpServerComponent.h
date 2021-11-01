#pragma once

#include <Component/Component.h>
namespace GameKeeper
{
	// 处理外部连接进来的session
	class NetworkListener;
	class TcpClientComponent;
	class TcpServerComponent : public Component
	{
	public:
		TcpServerComponent() {}

		~TcpServerComponent() {}

        int GetPriority() { return 2; }
	protected:
		bool Awake() override;
		void Start() override;
	private:
        std::string mHostIp;
		std::set<std::string> mWhiteList;    //白名单
        std::vector<NetworkListener *> mListeners;
	};
}