#pragma once
#include<Object/GameObject.h>
#include<Manager/NetProxyManager.h>
namespace Sentry
{
	class ProxyManager : public NetProxyManager
	{
	public:
		ProxyManager() { }
		~ProxyManager() { }
	private:
		std::string mProxyIP;
		unsigned short mPorxyPort;
		std::string mProxyAddress;
	};
}