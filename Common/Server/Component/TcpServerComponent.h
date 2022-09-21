#pragma once

#include"Component/Component.h"
namespace Sentry
{
	// 处理外部连接进来的session
    class SocketProxy;
	struct ListenConfig;
	class TcpServerListener;
	class InnerNetComponent;
    class TcpServerComponent : public Component, public IComplete
	{
	 public:
		TcpServerComponent() = default;
		~TcpServerComponent() final = default;
	public:
        bool AddBlackList(const std::string & ip);
        bool AddWhiteList(const std::string & ip);
        bool OnListenConnect(std::shared_ptr<SocketProxy> socket);
    private:
		bool LateAwake() final;
        void OnComplete() final;
        bool LoadServerConfig();
        void OnAllServiceStart() final;
        TcpServerListener * GetListener(const std::string & name);
	 private:
        std::set<std::string> mBlackList;    //黑名单
        std::set<std::string> mWhiteList;    //白名单
        std::unordered_map<std::string, const ListenConfig *> mListenConfigs;
    };
}