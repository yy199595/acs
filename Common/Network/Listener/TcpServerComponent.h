#pragma once

#include <Component/Component.h>
namespace Sentry
{
	// 处理外部连接进来的session
    struct ListenConfig;
	class NetworkListener;
	class RpcClientComponent;
    class TcpServerComponent : public Component, public IStart
	{
	public:
		TcpServerComponent() = default;
		~TcpServerComponent() final =default;

    public:
        const std::string & GetHostIp() const { return this->mHostIp;}
        const NetworkListener * GetListener(const std::string & name);
        void GetListeners(std::vector<const NetworkListener *> & listeners);
	protected:
		bool Awake() final;
		bool LateAwake() final;
        void OnStart() final;
	private:
        std::string mHostIp;
		std::set<std::string> mWhiteList;    //白名单
        std::vector<ListenConfig *> mListenerConfigs;
        std::vector<NetworkListener *> mListeners;
	};
}