#pragma once

#include <Manager/NetProxyManager.h>
#include <Object/GameObject.h>

namespace Sentry
{
    class ProxyManager : public NetProxyManager
    {
    public:
        ProxyManager() {}

        ~ProxyManager() {}

    private:
        std::string mProxyIP;
        unsigned short mPorxyPort;
        std::string mProxyAddress;
    };
}// namespace Sentry