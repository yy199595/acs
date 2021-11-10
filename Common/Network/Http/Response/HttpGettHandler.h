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

        void Clear() final;

        HttpMethodType GetType() final { return HttpMethodType::GET; }

		//bool SplitParameter(std::unordered_map<std::string, std::string> & parames);

        size_t ReadFromStream(std::string & stringBuf) override;

		bool OnReceiveHead(asio::streambuf & buf) override;

        const std::string & GetPath() override { return this->mPath; }

        void OnReceiveBody(asio::streambuf & streamBuf) final { assert(false); };

    private: // 请求参数
        std::string mPath;
		std::string mVersion;
    };
}
#endif //GameKeeper_HTTPREMOTEGETREQUEST_H
