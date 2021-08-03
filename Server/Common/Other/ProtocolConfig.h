#pragma once

#include<string>

namespace Sentry
{
    class ProtocolConfig
    {
    public:
        ProtocolConfig(const unsigned short id, const std::string &service, const std::string &method,
                       const std::string &request, const std::string &response, bool client = false)
                : IsClientMessage(client), MethodName(method), MethodId(id),
                  ServiceName(service), RequestMsgName(request), ResponseMsgName(response) {}

    public:
        const bool IsClientMessage;
        const std::string MethodName;
        const unsigned short MethodId;
        const std::string ServiceName;
        const std::string RequestMsgName;
        const std::string ResponseMsgName;
    };
}