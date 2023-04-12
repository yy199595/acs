//
// Created by yjz on 2022/10/27.
//

#ifndef APP_HTTPREQUEST_H
#define APP_HTTPREQUEST_H
#include<string>
#include<fstream>
#include"httpHead.h"
#include<unordered_map>
#include<rapidjson/document.h>
#include"Lua/Engine/Define.h"
#include"Proto/Message/ProtoMessage.h"
namespace Http
{
    class Parameter
    {
    public:
        explicit Parameter(const std::string & content);
    public:
        bool Get(std::vector<std::string> & keys);
        bool Get(const std::string & key, std::string & value);
    private:
        std::unordered_map<std::string, std::string> mParameters;
    };


    class Request : public IStream, public Tcp::ProtoMessage
    {
    public:
        explicit Request(const char * method, const std::string & from);
    public:
        inline const Head & Header() const { return this->mHead; }
		inline const std::string & From() const { return this->mFrom; }
		inline const std::string & Method() const { return this->mMethod; }
    public:
        virtual bool WriteLua(lua_State * lua) const = 0;       
        virtual bool WriteDocument(rapidjson::Document * document) const = 0;
    public:
		bool SetUrl(const std::string & url);
		void SetAsync(bool async) { this->mAsync = async;}
		void SetTimeout(int second) { this->mTimeout = second; }
        inline const std::string & Url() const { return this->mUrl; }
        inline const std::string & Host() const { return this->mHost; }
        inline const std::string & Port() const { return this->mPort; }
        inline const std::string & Path() const { return this->mPath; }
    public:
		bool Async() const { return this->mAsync;}
		int Timeout() const { return this->mTimeout;}
        const std::string & Content() const { return this->mContent;}
    public:
        int OnRead(std::istream &buffer) final;
        int OnWrite(std::ostream &buffer) final;
        int Serialize(std::ostream &os) final;
    protected:
        virtual int OnReadContent(std::istream & buffer) = 0;
        virtual int OnWriteContent(std::ostream & buffer) = 0;
    protected:
        Head mHead;
		bool mAsync;
		int mTimeout;
        std::string mUrl;
        std::string mHost;
        std::string mPath;
        std::string mPort;
		std::string mFrom;
		DecodeState mState;
		std::string mVersion;
        std::string mProtocol;
        const std::string mMethod;
    protected:
        std::string mContent;
    };
}


namespace Http
{
    class GetRequest : public Request
    {
    public:
		GetRequest(): Request("GET", "") { }
		explicit GetRequest(const std::string & from) :Request("GET", from) { }
    protected:
        void OnComplete() final { }
        int OnReadContent(std::istream &buffer) final;
        int OnWriteContent(std::ostream &buffer) final;
    protected:
        bool WriteLua(lua_State* lua) const final;
        bool WriteDocument(rapidjson::Document* document) const final;
    };

}

namespace Http
{
    class PostRequest : public Request
    {
    public:
		PostRequest() : Request("POST", "") { }
		explicit PostRequest(const std::string & from) : Request("POST", from) { }
    public:
        void Str(const std::string & str);
        void Json(const std::string & json);
        void Json(const char * str, size_t size);
    public:
        void OnComplete() final;
        int OnReadContent(std::istream &buffer) final;
        int OnWriteContent(std::ostream &buffer) final;
    protected:
        bool WriteLua(lua_State* lua) const final;
        bool WriteDocument(rapidjson::Document* document) const final;
    };
}

namespace Http
{
    extern std::shared_ptr<Request> New(const std::string & method, const std::string & from);
}


#endif //APP_HTTPREQUEST_H
