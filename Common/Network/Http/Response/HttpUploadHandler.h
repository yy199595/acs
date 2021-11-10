//
// Created by zmhy0073 on 2021/11/10.
//

#ifndef GAMEKEEPER_HTTPUPLOADHANDLER_H
#define GAMEKEEPER_HTTPUPLOADHANDLER_H
#include "HttpRequestHandler.h"
#include <fstream>
namespace GameKeeper
{
    class HttpUploadComponent;
    class HttpUploadHandler : public HttpRequestHandler
    {
    public:
        explicit HttpUploadHandler() = default;
        ~HttpUploadHandler() override = default;
    public:
        void Clear() final;
        const std::string & GetPath() final;
        bool OnReceiveHead(asio::streambuf & buf) final;
        size_t ReadFromStream(std::string & stringBuf) final;
        bool OnReceiveBody(asio::streambuf &streamBuf) final;
        HttpMethodType GetType() final { return HttpMethodType::POST; }
    private:
        std::string mPath;
        size_t mWriteBytes;
        std::string mVersion;
        std::ofstream mFstream;
    };
}

#endif //GAMEKEEPER_HTTPUPLOADHANDLER_H
