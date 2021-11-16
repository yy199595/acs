#pragma once

#include <string>

namespace GameKeeper
{
    class ProtocolConfig
    {
    public:
        int Timeout;
        bool IsAsync;
        int MethodId;
        std::string Method;
        std::string Service;
        std::string Request;
        std::string Response;
    };
}// namespace GameKeeper