//
// Created by zmhy0073 on 2021/11/1.
//

#ifndef GameKeeper_HTTPLOCALREQUEST_H
#define GameKeeper_HTTPLOCALREQUEST_H
#include <Http/HttpHandlerBase.h>
namespace GameKeeper
{
    class HttpLocalSession;

    class HttpComponent;

    class HttpRequest : public HttpHandlerBase
    {
    public:
        explicit HttpRequest(HttpComponent *component);

        ~HttpRequest() override = default;

    public:

        virtual void Clear() override;

        const std::string &GetError() const { return this->mError; }

        const std::string &GetVersion() const { return this->mVersion; }

        const std::string &GetPath() const { return this->mPath; }

        const std::string &GetHost() const { return this->mHost; }

        const std::string &GetPort() const { return this->mPort; }

        HttpStatus GetHttpCode() const { return (HttpStatus) mHttpCode; }

        bool ParseUrl(const std::string &url);

        virtual void OnReceiveBody(asio::streambuf & buf) = 0;

        bool WriterToBuffer(std::ostream &os) final;
    protected:

        virtual void WriteHead(std::ostream & os) = 0;
        virtual bool WriteBody(std::ostream & os) = 0;

    public:
        bool OnReceiveHead(asio::streambuf &buf) override;

    protected:
        int mHttpCode;
        std::string mError;
        std::string mVersion;
        HttpComponent *mHttpComponent;
    private:
        std::string mPath;
        std::string mHost;
        std::string mPort;
        size_t mWriteCount;
    };
}
#endif //GameKeeper_HTTPLOCALREQUEST_H
