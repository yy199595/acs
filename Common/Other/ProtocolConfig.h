#pragma once

#include <string>

namespace GameKeeper
{
    class ProtocolConfig
    {
    public:
        bool IsAsync;
        std::string Method;
        std::string Service;
        std::string Request;
        std::string Response;
    };
}// namespace GameKeeper