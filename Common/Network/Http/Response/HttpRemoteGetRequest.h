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

		bool SplitParameter(std::unordered_map<std::string, std::string> & parames);

        HttpMethodType GetMethodType() override { return HttpMethodType::GET; }

		const std::string & GetParamater() { return this->mParamater; }

    protected:

        bool OnReceiveBody(asio::streambuf &buf) override;

        bool OnReceiveHeard(asio::streambuf &buf, size_t size) override;

	private:
		bool ParseUrl(const std::string & path);

    private: // 请求参数
		std::string mParamater;		
    };
}
#endif //GameKeeper_HTTPREMOTEGETREQUEST_H
