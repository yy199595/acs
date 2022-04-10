#pragma once

#include <Component/Component.h>
namespace Sentry
{
	// 处理外部连接进来的session
	struct ListenConfig;
	class NetworkListener;
	class RpcClientComponent;
	class TcpServerComponent : public Component, public IComplete
	{
	 public:
		TcpServerComponent() = default;
		~TcpServerComponent() final = default;

	 public:
		const std::string& GetHostIp() const
		{
			return this->mHostIp;
		}
		const ListenConfig * GetTcpConfig(const std::string & name);
		void GetListenConfigs(std::vector<const ListenConfig*>& configs);
		std::string GetTcpAddress(const std::string & name);
	 protected:
		bool Awake() final;
		bool LateAwake() final;
		void OnAllServiceStart() final;
	 private:
		std::string mHostIp;
		std::set<std::string> mWhiteList;    //白名单
		std::vector<ListenConfig*> mListenerConfigs;
		std::vector<NetworkListener*> mListeners;
	};
}