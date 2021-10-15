#pragma once

#include <Component/Component.h>
namespace Sentry
{
	// 处理外部连接进来的session
	class TcpNetSessionComponent;
	class TcpServerComponent : public Component, public ITcpContextUpdate
	{
	public:
		TcpServerComponent() {}

		~TcpServerComponent() {}

	public:
		const std::string &GetAddress() { return mListenAddress; }

	protected:
		bool Awake() override;
		void Start() override;
		void OnTcpContextUpdate(AsioContext & io) final;

	private:
		bool mIsAccept;
        int mMaxConnectCount;
		std::string mListenerIp;    //监听的ip
		std::string mListenAddress;            //地址
		unsigned short mListenerPort;
		AsioTcpAcceptor *mBindAcceptor;
		TcpNetSessionComponent *mDispatchManager;
		std::set<std::string> mWhiteList;    //白名单
	};
}