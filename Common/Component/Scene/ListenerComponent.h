#pragma once

#include <Component/Component.h>
namespace Sentry
{
	// 处理外部连接进来的session
	class NetSessionComponent;
	class ListenerComponent : public Component, public INetSystemUpdate
	{
	public:
		ListenerComponent() {}

		~ListenerComponent() {}

	public:
		const std::string &GetAddress() { return mListenAddress; }

	protected:
		bool Awake() override;
		void Start() override;
		void OnNetSystemUpdate(AsioContext & io) final;

	private:
		bool mIsAccept;
        int mMaxConnectCount;
		std::string mListenerIp;    //监听的ip
		std::string mListenAddress;            //地址
		unsigned short mListenerPort;
		AsioTcpAcceptor *mBindAcceptor;
		NetSessionComponent *mDispatchManager;
		std::set<std::string> mWhiteList;    //白名单
	};
}