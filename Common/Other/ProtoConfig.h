#pragma once

#include <string>

namespace GameKeeper
{
    class ProtoConfig
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