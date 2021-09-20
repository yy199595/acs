#pragma once

#include<Scene/NetProxyComponent.h>
namespace Sentry
{
    class ProxyManager : public NetProxyComponent
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