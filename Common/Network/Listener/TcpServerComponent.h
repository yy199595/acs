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
		bool StartListen(const std::string & name);
	private:
		bool LateAwake() final;
		bool LoadServerConfig();
		void OnAllServiceStart() final;
	 private:
		std::set<std::string> mWhiteList;    //白名单
		std::vector<std::shared_ptr<NetworkListener>> mListeners;
	};
}