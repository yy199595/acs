//
// Created by zmhy0073 on 2021/11/4.
//

#ifndef GAMEKEEPER_HTTPPOSTHANDLER_H
#define GAMEKEEPER_HTTPPOSTHANDLER_H
#include "HttpRequestHandler.h"
#include <Http/Content/HttpReadContent.h>
namespace GameKeeper
{
    class HttpComponent;
    class HttpReadContent;
    class HttpPostHandler : public HttpRequestHandler
    {
    public:
        explicit HttpPostHandler(HttpComponent *component);
        ~HttpPostHandler() override = default;
    public:
        void Clear() final;
        const std::string & GetPath() final;
        bool OnReceiveHead(asio::streambuf & buf) final;
        bool OnReceiveBody(asio::streambuf &streamBuf) final;
        HttpReadContent * GetContent() final { return this->mContent; }
        HttpMethodType GetType() final { return HttpMethodType::POST; }

    private:
		std::string mPath;
		std::string mVersion;
        HttpReadContent * mContent;
    };
}
#endif //GAMEKEEPER_HTTPPOSTHANDLER_H
