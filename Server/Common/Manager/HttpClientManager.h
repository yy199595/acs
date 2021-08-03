
#pragma once

#include "Manager.h"

namespace SoEasy
{
    class HttpClientManager : public Manager
    {
    public:
        HttpClientManager() {}

        ~HttpClientManager() {}

    public:

    private:
        AsioContext mHttpAsioContext;

        class CoroutineManager *mCorManager;
    };
}