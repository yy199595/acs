//
// Created by zmhy0073 on 2022/1/19.
//

#ifndef GAMEKEEPER_HTTPASYNCREQUEST_H
#define GAMEKEEPER_HTTPASYNCREQUEST_H
#include<string>
#include<asio.hpp>
#include<fstream>
#include<unordered_map>
#include"Json/JsonWriter.h"
#include"Json/JsonReader.h"
#include<Network/Http/Http.h>
#include"Network/Proto/ProtoMessage.h"

namespace Sentry
{
	class HttpAsyncRequest : public Tcp::ProtoMessage
    {
	public:
		HttpAsyncRequest(const std::string& method);
	public:
		bool AddHead(const std::string & key, int value);
		bool AddHead(const std::string & key, const std::string & value);
    public:
		int Serailize(std::ostream &os) final;
		const std::string & GetHost() { return this->mHost;}
        const std::string & GetPort() { return this->mPort;}
	protected:
		bool ParseUrl(const std::string & url);
		virtual void WriteBody(std::ostream & os) = 0;
	private:
        std::string mHost;
        std::string mPort;
        std::string mPath;
		const std::string mMethod;
        asio::streambuf mSendStream;
        std::unordered_map<std::string, std::string> mHeadMap;
    };
}

namespace Sentry
{
	class HttpGetRequest : public HttpAsyncRequest
	{
	public:
		HttpGetRequest();
	public:
		static std::shared_ptr<HttpGetRequest> Create(const std::string & url);
	protected:
		void WriteBody(std::ostream & os) final;
	};
}

namespace Sentry
{
	class HttpPostRequest : public HttpAsyncRequest
	{
	public:
		HttpPostRequest();
	public:
		void AddBody(const std::string & content);
		void AddBody(const char * data, size_t size);
		static std::shared_ptr<HttpPostRequest> Create(const std::string & url);
	protected:
		void WriteBody(std::ostream & os) final;
	private:
		std::string mBody;
	};
}

namespace Sentry
{
    enum class HttpDecodeState
    {
        FirstLine,
        HeadLine,
        Content,
        Finish
    };
    class IHttpContent
    {
    public:
        virtual const std::string & GetContent() const = 0;
        virtual HttpStatus OnReceiveData(asio::streambuf & streamBuffer) = 0;
    };
}

namespace Sentry
{
    class HttpAsyncResponse : public IHttpContent
    {
    public:
        HttpAsyncResponse();
    public:
        HttpStatus OnReceiveData(asio::streambuf &streamBuffer) final;
        HttpStatus GetHttpCode() { return (HttpStatus)this->mHttpCode;}
        const std::string & GetContent() const final { return this->mContent;}
	public:
		bool ToJson(std::string & json);
		bool GetHead(const std::string & key, std::string & value);
    private:
        int mHttpCode;
        std::string mContent;
        std::string mVersion;
        std::string mHttpError;
        HttpDecodeState mState;
        size_t mContentLength;
        std::unordered_map<std::string, std::string> mHeadMap;
    };
}

namespace Sentry
{
	class HttpHandlerRequest : public IHttpContent
	{
	 public:
		HttpHandlerRequest(const std::string & address);
	 public:
		HttpStatus OnReceiveData(asio::streambuf& streamBuffer) final;
	 public:
		const std::string& GetPath() const { return this->mPath; }
		const std::string& GetMethod() const { return this->mMethod; }
		const std::string& GetAddress() const { return this->mAddress; }
		const std::string& GetContent() const final { return this->mContent; }
		bool GetHeadContent(const std::string& key, std::string& value) const;
	 private:
		std::string mPath;
		std::string mMethod;
		std::string mContent;
		std::string mVersion;
		std::string mAddress;
		HttpDecodeState mState;
		std::unordered_map<std::string, std::string> mHeadMap;
	};
}

namespace Sentry
{
	class HttpHandlerResponse : public Tcp::ProtoMessage
	{
	 public:
		HttpHandlerResponse();
		~HttpHandlerResponse();
	 public:
		void SetCode(HttpStatus code);
		void WriteFile(std::fstream * fs);
		void WriteString(const std::string & content);
		bool AddHead(const char * key, const std::string & value);
	 private:
		int Serailize(std::ostream &os) final;
	 private:
		size_t mCount;
		HttpStatus mCode;
		size_t mContentSize;
		std::string mContent;
		std::fstream * mFstream;
		std::unordered_map<std::string, std::string> mHeadMap;
	};
}
#endif //GAMEKEEPER_HTTPASYNCREQUEST_H
