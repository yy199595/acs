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
		const ListenConfig * GetTcpConfig(const std::string & name);
		void GetListenConfigs(std::vector<const ListenConfig*>& configs);
		std::string GetTcpAddress(const std::string & name);

	public:
		bool StartListen(const std::string & name);
	 protected:
		bool LateAwake() final;
		void OnAllServiceStart() final;

	private:
		bool LoadServerConfig();
	 private:
		std::set<std::string> mWhiteList;    //白名单
		std::vector<ListenConfig*> mListenerConfigs;
		std::vector<NetworkListener*> mListeners;
	};
}