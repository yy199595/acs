#pragma once

#include<Scene/SceneNetProxyComponent.h>
namespace Sentry
{
    class ProxyManager : public SceneNetProxyComponent
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