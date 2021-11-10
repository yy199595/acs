//
// Created by zmhy0073 on 2021/11/10.
//

#ifndef GAMEKEEPER_HTTPUPLOADSESSION_H
#define GAMEKEEPER_HTTPUPLOADSESSION_H


#include "HttpSessionBase.h"
namespace GameKeeper
{
    class HttpUploadComponent;
    class HttpUploadSession : public HttpSessionBase
    {
    public:
        explicit HttpUploadSession(HttpUploadComponent * socketHandler);
        ~HttpUploadSession() final = default;
    public:
        void Start(SocketProxy * socketProxy);
        SocketType GetSocketType() final { return SocketType::RemoteSocket; }
    public:
        void Clear() final;
    protected:
        void OnWriterAfter(XCode code) final;
        void OnReceiveHeadAfter(XCode code) final;
        bool WriterToBuffer(std::ostream &) final;
        void OnReceiveHeard(asio::streambuf & buf) final;

    private:
        void StartReceiveBody();
        void SetCode(XCode code);
        void ReadBodyCallback(const asio::error_code & err, size_t size);
    private:
        size_t mWriterCount;
        unsigned int mCorId;
    private:
        std::string mMethod;
        HttpUploadComponent * mUploadComponent;
        class HttpReadFileContent * mFileContent;
    };
}

#endif //GAMEKEEPER_HTTPUPLOADSESSION_H
