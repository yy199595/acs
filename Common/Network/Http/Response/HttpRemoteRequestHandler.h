//
// Created by zmhy0073 on 2021/11/1.
//

#ifndef GameKeeper_HTTPREMOTEREQUEST_H
#define GameKeeper_HTTPREMOTEREQUEST_H
#include <Network/Http/HttpHandlerBase.h>
#include <Network/Http/Content/HttpWriteContent.h>

namespace GameKeeper
{
    class HttpServiceConfig;
    class HttpRemoteSession;
    class HttpClientComponent;

    class HttpRemoteRequestHandler : public HttpHandlerBase
    {
    public:
        explicit HttpRemoteRequestHandler(HttpClientComponent *component, HttpRemoteSession *session);

         virtual ~HttpRemoteRequestHandler() ;

    public:
        void SetCode(HttpStatus code);

        bool SetContent(HttpWriteContent * httpContent);

        bool SetHeard(const std::string &key, const std::string &val);

    public:

        bool WriterToBuffer(std::ostream &os) override;

        XCode GetErrorCode() const { return this->mCode; }

        HttpRemoteSession * GetSession() { return this->mHttpSession; }

        const HttpServiceConfig * GetHttpConfig() const { return this->mHttpConfig;}

        void OnWriterAfter(XCode code) override;

    protected:

        virtual bool ParseUrl(const std::string & path);

	protected:
        XCode mCode;
    protected:
        HttpRemoteSession *mHttpSession;
        const HttpServiceConfig * mHttpConfig;
        HttpClientComponent *mHttpComponent;
    private:
#ifdef __DEBUG__
      long long mStartTime;
#endif
        int mWriteCount;
        HttpStatus mHttpCode;
        HttpWriteContent * mHttpContent;
        std::unordered_map<std::string, std::string> mHeardMap;
    };
}

#endif //GameKeeper_HTTPREMOTEREQUEST_H
