#pragma once

#include <string>

namespace GameKeeper
{
    class ProtocolConfig
    {
    public:
        bool IsAsync;
        std::string Method;
        unsigned short MethodId;
        std::string ServiceName;
        std::string RequestMessage;
        std::string ResponseMessage;
        std::string RequestHandler;
        std::string ResponseHandler;
    };

    class HttpServiceConfig
    {
    public:
        bool IsAsync;
        std::string Path;
        std::string Method;
        std::string Service;
        std::vector<std::string> HeardFields;
    };
}// namespace GameKeeper