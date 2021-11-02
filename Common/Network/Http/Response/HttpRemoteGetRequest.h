//
// Created by zmhy0073 on 2021/11/1.
//

#ifndef GameKeeper_HTTPREMOTEGETREQUEST_H
#define GameKeeper_HTTPREMOTEGETREQUEST_H
#include "HttpContent.h"
#include "HttpRemoteRequest.h"

namespace GameKeeper
{
    class HttpRemoteGetRequest : public HttpRemoteRequest
    {
    public:
      using HttpRemoteRequest::HttpRemoteRequest;
    public:
        const std::string & GetPath() { return this->mPath;}

        bool GetParameter(const std::string & key, std::string & val);

        HttpMethodType GetMethodType() override { return HttpMethodType::GET; }
    protected:

        bool OnReceiveBody(asio::streambuf &buf) override;

        bool OnReceiveHeard(asio::streambuf &buf, size_t size) override;

    private: // 请求参数
        std::unordered_map<std::string, std::string> mParameMap;
    };
}
#endif //GameKeeper_HTTPREMOTEGETREQUEST_H
