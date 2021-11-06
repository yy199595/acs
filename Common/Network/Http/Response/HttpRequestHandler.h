//
// Created by zmhy0073 on 2021/11/1.
//

#ifndef GameKeeper_HTTPREMOTEREQUEST_H
#define GameKeeper_HTTPREMOTEREQUEST_H
#include <Network/Http/HttpHandlerBase.h>
namespace GameKeeper
{
	class HttpReadContent;
	class HttpWriteContent;
    class HttpServiceConfig;
    class HttpRemoteSession;
    class HttpClientComponent;
	

    class HttpRequestHandler : public HttpHandlerBase
    {
    public:
        explicit HttpRequestHandler(HttpClientComponent *component, HttpRemoteSession *session);

         virtual ~HttpRequestHandler() override;

    public:
        void SetCode(HttpStatus code);

        bool SetContent(HttpWriteContent * httpContent);

        bool SetHeard(const std::string &key, const std::string &val);

        virtual const std::string & GetPath() = 0;

        virtual const std::string & GetParameter() = 0;

    public:

        bool WriterToBuffer(std::ostream &os) override;

        XCode GetErrorCode() const { return this->mCode; }

        HttpRemoteSession * GetSession() { return this->mHttpSession; }

        const HttpServiceConfig * GetHttpConfig() const { return this->mHttpConfig;}

        void OnWriterAfter(XCode code) override;

	protected:
        XCode mCode;
    protected:
        HttpRemoteSession *mHttpSession;
        HttpClientComponent *mHttpComponent;
		const HttpServiceConfig * mHttpConfig;
    private:
#ifdef __DEBUG__
      long long mStartTime;
#endif
        int mWriteCount;
        HttpStatus mHttpCode;
		std::string mVersion;
        HttpWriteContent * mHttpContent;
        std::unordered_map<std::string, std::string> mHeardMap;
    };
}

#endif //GameKeeper_HTTPREMOTEREQUEST_H
