#pragma once

#include <string>

namespace Sentry
{
    class ProtocolConfig
    {
    public:
        bool Async;
        std::string Method;
        unsigned short Id;
        std::string Service;
        std::string Request;
        std::string Response;
    };
}// namespace Sentry