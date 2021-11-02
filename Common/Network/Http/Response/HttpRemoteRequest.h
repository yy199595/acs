//
// Created by zmhy0073 on 2021/11/1.
//

#ifndef GameKeeper_HTTPREMOTEREQUEST_H
#define GameKeeper_HTTPREMOTEREQUEST_H
#include <Network/Http/HttpHandlerBase.h>
#include <Network/Http/Response/HttpContent.h>



namespace GameKeeper
{
    class HttpRemoteSession;
    class HttpClientComponent;

    class HttpRemoteRequest : public HttpHandlerBase
    {
    public:
        HttpRemoteRequest(HttpClientComponent *component, HttpRemoteSession *session);

        virtual ~HttpRemoteRequest();

    public:
        void SetCode(HttpStatus code);

        bool SetContent(HttpContent * httpContent);

        bool SetHeard(const std::string &key, const std::string &val);

        virtual HttpMethodType GetMethodType() = 0;
    public:

        bool WriterToBuffer(std::ostream &os) override;

        const std::string & GetPath() { return this->mPath;}

        const std::string & GetVersion() { return this->mVersion;}

        HttpRemoteSession * GetSession() { return this->mHttpSession; }

    protected:

        void NoticeMainThread();

        bool OnSessionError(const asio::error_code &code) override;

    protected:
        std::string mPath;
        std::string mVersion;
        HttpRemoteSession *mHttpSession;
        HttpClientComponent *mHttpComponent;
    private:
        int mWriteCount;
        HttpStatus mHttpCode;
        HttpContent * mHttpContent;
        std::unordered_map<std::string, std::string> mHeardMap;
    };
}

#endif //GameKeeper_HTTPREMOTEREQUEST_H
