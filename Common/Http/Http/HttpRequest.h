//
// Created by yjz on 2022/10/27.
//

#ifndef APP_HTTPREQUEST_H
#define APP_HTTPREQUEST_H
#include<string>
#include"Comman.h"
#include<unordered_map>
#include"Message/ProtoMessage.h"
namespace Http
{
    class Request : public IStream, public Tcp::ProtoMessage
    {
    public:
        Request(const char * method);
    public:
        Head & GetHead() { return this->mHead; }
        const std::string & Method() const { return this->mMethod; }
    public:
        bool SetUrl(const std::string & url);
        const std::string & GetHost() const { return this->mHost; }
        const std::string & GetPort() const { return this->mPath; }
    protected:
        int OnRead(std::istream &buffer) final;
        int OnWrite(std::ostream &buffer) final;
        int Serailize(std::ostream &os) final;
    protected:
        virtual int OnReadContent(std::istream & buffer) = 0;
        virtual int OnWriteContent(std::ostream & buffer) = 0;
    protected:
        Head mHead;
        State mState;
        std::string mHost;
        std::string mPath;
        std::string mUrl;
        std::string mPort;
        std::string mVersion;
        std::string mProtocol;
        const std::string mMethod;
    };
}

namespace Http
{
    class GetRequest : public Request
    {
    public:
        GetRequest(const std::string & url) :Request("GET") { }
    public:
        int OnReadContent(std::istream &buffer) final;
        int OnWriteContent(std::ostream &buffer) final;
    public:
        bool GetParameter(const std::string & key, std::string & value);
    private:
        std::unordered_map<std::string, std::string> mParameters;
    };
}

namespace Http
{
    class PostRequest : public Request
    {
    public:
        PostRequest() : Request("POST") { }
    public:
        int OnReadContent(std::istream &buffer) final;
        int OnWriteContent(std::ostream &buffer) final;
    public:
        const std::string & Content() const { return this->mContent;}
    private:
        std::string mContent;
    };
}


#endif //APP_HTTPREQUEST_H
