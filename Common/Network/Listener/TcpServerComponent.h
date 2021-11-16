#pragma once

#include <Component/Component.h>
namespace GameKeeper
{
	// 处理外部连接进来的session
	class NetworkListener;
	class RpcComponent;
	class TcpServerComponent : public Component
	{
	public:
		TcpServerComponent() = default;
		~TcpServerComponent() final =default;

    public:
        int GetPriority() final { return 2; }
        const std::string & GetHostIp() const { return this->mHostIp;}
        void GetListeners(std::vector<const NetworkListener *> & listeners);
	protected:
		bool Awake() override;
		void Start() override;
	private:
        std::string mHostIp;
		std::set<std::string> mWhiteList;    //白名单
        std::vector<NetworkListener *> mListeners;
	};
}