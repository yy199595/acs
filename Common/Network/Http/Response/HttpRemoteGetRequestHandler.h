//
// Created by zmhy0073 on 2021/11/1.
//

#ifndef GameKeeper_HTTPREMOTEGETREQUEST_H
#define GameKeeper_HTTPREMOTEGETREQUEST_H
#include "HttpRemoteRequestHandler.h"

namespace GameKeeper
{
    class HttpRemoteGetRequestHandler : public HttpRemoteRequestHandler
    {
    public:
        using HttpRemoteRequestHandler::HttpRemoteRequestHandler;
        ~HttpRemoteGetRequestHandler() override = default;
    public:
        const std::string & GetPath() { return this->mPath;}

		bool SplitParameter(std::unordered_map<std::string, std::string> & parames);

		const std::string & GetParamater() { return this->mParamater; }

    protected:

        void OnReceiveBody(asio::streambuf &buf) override { }

        bool OnReceiveHeard(asio::streambuf &buf, size_t size) override;

	private:
		bool ParseUrl(const std::string & path);

    private: // 请求参数
		std::string mParamater;		
    };
}
#endif //GameKeeper_HTTPREMOTEGETREQUEST_H
