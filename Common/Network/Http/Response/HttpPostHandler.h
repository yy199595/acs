//
// Created by zmhy0073 on 2021/11/4.
//

#ifndef GAMEKEEPER_HTTPPOSTHANDLER_H
#define GAMEKEEPER_HTTPPOSTHANDLER_H
#include "HttpRequestHandler.h"
#include <Network/Http/Content/HttpReadContent.h>
namespace GameKeeper
{
    class HttpComponent;
    class HttpPostHandler : public HttpRequestHandler
    {
    public:
        explicit HttpPostHandler(HttpComponent *component);
        ~HttpPostHandler() override = default;
    public:
        void Clear() final;
        const std::string & GetPath() final;
        bool OnReceiveHead(asio::streambuf & buf) final;
        size_t ReadFromStream(std::string & stringBuf) final;
        bool OnReceiveBody(asio::streambuf &streamBuf) final;
        HttpMethodType GetType() final { return HttpMethodType::POST; }
    private:
		std::string mPath;
		size_t mDataLength;
		std::string mVersion;
    };
}
#endif //GAMEKEEPER_HTTPPOSTHANDLER_H
