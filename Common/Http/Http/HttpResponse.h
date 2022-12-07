//
// Created by yjz on 2022/10/27.
//

#ifndef APP_HTTPRESPONSE_H
#define APP_HTTPRESPONSE_H
#include"httpHead.h"
#include"Json/JsonWriter.h"
#include"Message/ProtoMessage.h"
namespace Http
{
    class Response : public IStream, public Tcp::ProtoMessage
    {
    public:
        Response();
    public:
        bool OnRead(std::istream &buffer) final;
        int OnWrite(std::ostream &buffer) final;
        int Serailize(std::ostream & buffer) final;
    public:
        void SetCode(HttpStatus code) { this->mCode = (int)code;}
        void Str(HttpStatus code, const std::string & str);
        void Json(HttpStatus code, Json::Writer & doc);
        void Json(HttpStatus code, const std::string & json);
        void Json(HttpStatus code, const char * str, size_t len);
    public:
        Head & Header() { return this->mHead; }
        HttpStatus Code() const { return (HttpStatus)this->mCode; }
        const std::string & GetError() const { return this->mError; }
        const std::string & Content() const { return this->mContent; }
    private:
        Head mHead;
        int mCode;
        std::string mError;
        std::string mVersion;
        std::string mContent;
        DecodeState mParseState;
    };
}


#endif //APP_HTTPRESPONSE_H
