//
// Created by zmhy0073 on 2021/11/1.
//

#ifndef GameKeeper_HTTPREMOTEREQUEST_H
#define GameKeeper_HTTPREMOTEREQUEST_H
#include <Network/Http/HttpHandlerBase.h>
#include <Network/Http/Content/HttpWriteContent.h>

namespace GameKeeper
{
    class HttpRemoteSession;
    class HttpClientComponent;

    class HttpRemoteRequestHandler : public HttpHandlerBase
    {
    public:
        explicit HttpRemoteRequestHandler(HttpClientComponent *component, HttpRemoteSession *session);

         ~HttpRemoteRequestHandler() ;

    public:
        void SetCode(HttpStatus code);

        bool SetContent(HttpWriteContent * httpContent);

        bool SetHeard(const std::string &key, const std::string &val);

    public:

        bool WriterToBuffer(std::ostream &os) override;

        void OnSessionError(const asio::error_code &code) override;

        const std::string & GetPath() { return this->mPath;}

        const std::string & GetVersion() { return this->mVersion;}

        HttpRemoteSession * GetSession() { return this->mHttpSession; }

		const std::string & GetMethodName() { return this->mMethod; }

		const std::string & GetServiceName() { return this->mService; }

        XCode GetErrorCode() { return this->mCode; }

        void OnWriterAfter() override;

        bool OnReceiveHeard(asio::streambuf &buf, size_t size) override;

    protected:

        virtual void SetCode(XCode code);

        virtual bool ParseUrl(const std::string & path);


	protected:
		std::string mMethod;
		std::string mService;
    protected:
        std::string mPath;
        std::string mVersion;
        HttpRemoteSession *mHttpSession;
        HttpClientComponent *mHttpComponent;
    private:
#ifdef __DEBUG__
      long long mStartTime;
#endif
        XCode mCode;
        int mWriteCount;
        HttpStatus mHttpCode;
        HttpWriteContent * mHttpContent;
        std::unordered_map<std::string, std::string> mHeardMap;
    };
}

#endif //GameKeeper_HTTPREMOTEREQUEST_H
