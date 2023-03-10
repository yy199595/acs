//
// Created by yjz on 2022/10/27.
//

#ifndef APP_HTTPREQUEST_H
#define APP_HTTPREQUEST_H
#include<string>
#include"httpHead.h"
#include<unordered_map>
#include"Lua/LuaInclude.h"
#include<rapidjson/document.h>
#include"Message/ProtoMessage.h"
namespace Http
{
    class Request : public IStream, public Tcp::ProtoMessage
    {
    public:
        Request(const char * method);
    public:
        inline const Head & Header() const { return this->mHead; }
        inline const std::string & Method() const { return this->mMethod; }
    public:
        virtual bool WriteLua(lua_State * lua) const = 0;       
        virtual bool WriteDocument(rapidjson::Document * document) const = 0;
    public:
        bool SetUrl(const std::string & url);
        inline const std::string & Url() const { return this->mUrl; }
        inline const std::string & Host() const { return this->mHost; }
        inline const std::string & Port() const { return this->mPort; }
        inline const std::string & Path() const { return this->mPath; }
    public:
        bool OnRead(std::istream &buffer) final;
        int OnWrite(std::ostream &buffer) final;
        int Serailize(std::ostream &os) final;
    protected:
        virtual bool OnReadContent(std::istream & buffer) = 0;
        virtual int OnWriteContent(std::ostream & buffer) = 0;
    protected:
        Head mHead;
        std::string mUrl;
        std::string mHost;
        std::string mPath;
        std::string mPort;
        DecodeState mState;
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
        GetRequest() :Request("GET") { }
    protected:
        bool OnReadContent(std::istream &buffer) final;
        int OnWriteContent(std::ostream &buffer) final;
    protected:
        bool WriteLua(lua_State* lua) const final;
        bool WriteDocument(rapidjson::Document* document) const final;
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
        void Str(const std::string & str);
        void Json(const std::string & json);
        void Json(const char * str, size_t size);
    public:
        bool OnReadContent(std::istream &buffer) final;
        int OnWriteContent(std::ostream &buffer) final;
    protected:
        bool WriteLua(lua_State* lua) const final;
        bool WriteDocument(rapidjson::Document* document) const final;
    public:
        const std::string & Content() const { return this->mContent;}
    private:
        std::string mContent;
    };
}

namespace Http
{
    extern std::shared_ptr<Request> New(const std::string & method);
}


#endif //APP_HTTPREQUEST_H
