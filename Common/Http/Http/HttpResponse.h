//
// Created by yjz on 2022/10/27.
//

#ifndef APP_HTTPRESPONSE_H
#define APP_HTTPRESPONSE_H
#include"httpHead.h"
#include"Message/ProtoMessage.h"
namespace Json
{
    class Writer;
}
namespace Http
{
    class IResponse : public IStream, public Tcp::ProtoMessage
    {
    public:
        void SetCode(HttpStatus code) { this->mCode = (int)code; }
        HttpStatus Code() const { return (HttpStatus)this->mCode; }
    private:
        int mCode;
    };


    class Response : public IResponse
    {
    public:
        Response();
    public:
        bool OnRead(std::istream &buffer) final;
        int OnWrite(std::ostream &buffer) final;
        int Serialize(std::ostream & buffer) final;
    public:
		void Str(HttpStatus code, const std::string & str);
        void Json(HttpStatus code, Json::Writer & doc);
		void Html(HttpStatus code, const std::string & html);
		void Json(HttpStatus code, const std::string & json);
        void Json(HttpStatus code, const char * str, size_t len);	
        void Content(HttpStatus code, const std::string& type, const std::string& str);
	public:
        Head & Header() { return this->mHead; }
        const std::string & GetError() const { return this->mError; }
        const std::string & GetContent() const { return this->mContent; }
    private:
        Head mHead;
        std::string mError;
        std::string mVersion;
        std::string mContent;
        DecodeState mParseState;
    };
}


namespace Http
{
    class FileResponse : public IResponse
    {
    public:
        bool OnRead(std::istream& buffer) final;
        int OnWrite(std::ostream& buffer) final;
        int Serialize(std::ostream& buffer) final;
    };
}

#endif //APP_HTTPRESPONSE_H
