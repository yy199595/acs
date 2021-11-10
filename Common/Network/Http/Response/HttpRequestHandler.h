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
    class HttpComponent;
	

    class HttpRequestHandler : public HttpHandlerBase
    {
    public:
        explicit HttpRequestHandler(HttpComponent *component);

         virtual ~HttpRequestHandler() override;

    public:

        void Clear() override;

        void SetResponseCode(HttpStatus code);

        void SetResponseContent(HttpWriteContent * httpContent);

        bool AddResponseHeard(const std::string &key, const std::string &val);

        virtual const std::string & GetPath() = 0;

        virtual size_t ReadFromStream(std::string & stringBuf) = 0;

        virtual bool OnReceiveBody(asio::streambuf & streamBuf) = 0;

#ifdef __DEBUG__
        long long GetStartTime() const { return this->mStartTime;}
#endif
    public:

        bool WriterToBuffer(std::ostream &os) override;

        HttpStatus GetResponseCode() { return this->mHttpCode; }

        const std::string & GetMethod() const { return this->mMethod;}

        const std::string & GetComponent() const { return this->mComponent;}

        const std::string & GetParamater() const { return this->mParamater;}

    private:
         void WriteHead(std::ostream & os);
         bool WriteBody(std::ostream & os);
    protected:
        std::string mMethod;
        std::string mComponent;
        std::string mParamater;
        HttpComponent *mHttpComponent;
		//const HttpServiceConfig * mHttpConfig;
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
