//
// Created by yjz on 2022/10/27.
//

#ifndef APP_HTTPRESPONSE_H
#define APP_HTTPRESPONSE_H
#include<fstream>
#include"httpHead.h"
#include"Proto/Message/ProtoMessage.h"
namespace Json
{
    class Writer;
}


namespace Http
{
    class IResponse : public IStream, public Tcp::ProtoMessage
    {
	 public:
		IResponse();
    public:
		Head & Header() { return this->mHead; }
		void SetCode(HttpStatus code) { this->mCode = (int)code; }
        HttpStatus Code() const { return (HttpStatus)this->mCode; }
		const std::string & GetError() const { return this->mError; }
	 protected:
		virtual int OnWriterContent(std::ostream & buff) = 0;
		virtual int OnReadContent(const char * str, size_t size) = 0;
		virtual HttpStatus GetCode() { return (HttpStatus)this->mCode; }
	 public:
		virtual size_t ContentSize() = 0;
		int OnRead(std::istream &buffer) final;
		int OnWrite(std::ostream &buffer) final;
		int Serialize(std::ostream &os) final { return this->OnWrite(os); }
	 protected:
        int mCode;
		Head mHead;
		std::string mError;
		std::string mVersion;
		DecodeState mParseState;
	};


    class DataResponse : public IResponse
    {
	 protected:
		int OnWriterContent(std::ostream &buff) final;
		int OnReadContent(const char *str, size_t size) final;
    public:
		void OnComplete() final { }
		void Str(HttpStatus code, const std::string & str);
        void Json(HttpStatus code, Json::Writer & doc);
		void Html(HttpStatus code, const std::string & html);
		void Json(HttpStatus code, const std::string & json);
        void Json(HttpStatus code, const char * str, size_t len);	
        void Content(HttpStatus code, const std::string& type, const std::string& str);
	public:
        const std::string & GetContent() const { return this->mContent; }
		size_t ContentSize() final { return this->mContent.size(); }
    private:
        std::string mContent;
    };
}


namespace Http
{
	class FileResponse : public IResponse
	{
	 public:
		FileResponse(const std::string & path);		
		~FileResponse();
	 public:
		void OnComplete() final;
		HttpStatus GetCode() final;
		int OnWriterContent(std::ostream &buff) final;
		int OnReadContent(const char *str, size_t size) final;
	 private:
		 size_t ContentSize() final;
	 private:
		size_t mCurSize;
		size_t mFileSize;
		const std::string mPath;
		std::ifstream * mInput;
		std::ofstream * mOutput;
	};
}

#endif //APP_HTTPRESPONSE_H
