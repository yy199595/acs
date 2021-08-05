
#pragma once

#include "Manager.h"
#include "ManagerInterface.h"

namespace Sentry
{
    class HttpClientManager : public Manager, public ISystemUpdate
    {
    public:
        HttpClientManager() {}

        ~HttpClientManager() {}

    public:
        bool OnInit() final;
        void OnSystemUpdate() final;

    public:
        XCode Get(const std::string & url, std::string & json, int timeout = 5);

    private:
        AsioContext mHttpAsioContext;

        class CoroutineManager *mCorManager;
    };
}