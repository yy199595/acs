#pragma once

#include <Component/Component.h>
namespace GameKeeper
{
	// 处理外部连接进来的session
    struct ListenConfig;
	class NetworkListener;
	class ProtoRpcClientComponent;
    class TcpServerComponent : public Component, public IStart
	{
	public:
		TcpServerComponent() = default;
		~TcpServerComponent() final =default;

    public:
        int GetPriority() final { return 2; }
        const std::string & GetHostIp() const { return this->mHostIp;}
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