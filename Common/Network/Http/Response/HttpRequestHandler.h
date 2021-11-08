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
        explicit HttpRequestHandler(HttpClientComponent *component);

         virtual ~HttpRequestHandler() override;

    public:

        void Clear() override;

        void SetResponseCode(HttpStatus code);

        void SetResponseContent(HttpWriteContent * httpContent);

        bool AddResponseHeard(const std::string &key, const std::string &val);

        virtual const std::string & GetPath() = 0;

        virtual size_t ReadFromStream(char * buffer, size_t size) = 0;

    public:

        bool WriterToBuffer(std::ostream &os) override;
        
        const HttpServiceConfig * GetHttpConfig() const { return this->mHttpConfig;}

    protected:
        HttpClientComponent *mHttpComponent;
		const HttpServiceConfig * mHttpConfig;
    private:
#ifdef __DEBUG__
      long long mStartTime;
#endif
        int mWriteCount;
        HttpStatus mHttpCode;
		std::string mVersion;
        HttpWriteContent * mResponseContent;
        std::unordered_map<std::string, std::string> mHeardMap;
    };
}

#endif //GameKeeper_HTTPREMOTEREQUEST_H
