//
// Created by zmhy0073 on 2021/11/1.
//

#ifndef GameKeeper_HTTPREMOTEGETREQUEST_H
#define GameKeeper_HTTPREMOTEGETREQUEST_H
#include "HttpRequestHandler.h"

namespace GameKeeper
{
    class HttpReadStringContent;
    class HttpGettHandler : public HttpRequestHandler
    {
    public:
        using HttpRequestHandler::HttpRequestHandler;
        ~HttpGettHandler() override = default;
    public:

        void Clear() final;

        HttpMethodType GetType() final { return HttpMethodType::GET; }

		//bool SplitParameter(std::unordered_map<std::string, std::string> & parames);
        
		bool OnReceiveHead(asio::streambuf & buf) override;

        const std::string & GetPath() override { return this->mPath; }

        bool OnReceiveBody(asio::streambuf & streamBuf) final;

        HttpReadContent * GetContent() final;

    private: // 请求参数
        std::string mPath;
		std::string mVersion;
        HttpReadStringContent * mContent;
    };
}
#endif //GameKeeper_HTTPREMOTEGETREQUEST_H
