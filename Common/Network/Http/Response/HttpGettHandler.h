//
// Created by zmhy0073 on 2021/11/1.
//

#ifndef GameKeeper_HTTPREMOTEGETREQUEST_H
#define GameKeeper_HTTPREMOTEGETREQUEST_H
#include "HttpRequestHandler.h"

namespace GameKeeper
{
    class HttpGettHandler : public HttpRequestHandler
    {
    public:
        using HttpRequestHandler::HttpRequestHandler;
        ~HttpGettHandler() override = default;
    public:

        HttpMethodType GetType() final { return HttpMethodType::GET; }

		bool SplitParameter(std::unordered_map<std::string, std::string> & parames);

        const std::string & GetParameter() override { return this->mParamater;}

		void OnReceiveHeardAfter(XCode code) override;

		bool OnReceiveHeard(asio::streambuf & buf) override;

        const std::string & GetPath() override { return this->mPath; }
    protected:

        void OnReceiveBodyAfter(XCode code) override { assert(false); }

		void OnReceiveBody(asio::streambuf &buf) override { assert(false); }

    private: // 请求参数
        std::string mPath;
		std::string mVersion;
        std::string mParamater;
    };
}
#endif //GameKeeper_HTTPREMOTEGETREQUEST_H
