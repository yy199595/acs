//
// Created by zmhy0073 on 2022/1/19.
//

#ifndef GAMEKEEPER_HTTPASYNCREQUEST_H
#define GAMEKEEPER_HTTPASYNCREQUEST_H
#include<string>
#include<asio.hpp>
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
		bool Serailize(std::ostream &os) final;
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
        virtual const std::string & GetContent() = 0;
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
        const std::string & GetContent() final { return this->mContent;}
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
		const std::string& GetPath() { return this->mPath; }
		const std::string& GetMethod() { return this->mMethod; }
		const std::string& GetAddress() { return this->mAddress; }
		const std::string& GetContent() final { return this->mContent; }
		bool GetHeadContent(const std::string& key, std::string& value);
	 private:
		std::string mPath;
		std::string mMethod;
		std::string mContent;
		std::string mVersion;
		std::string mAddress;
		HttpDecodeState mState;
		size_t mContentLength;
		std::unordered_map<std::string, std::string> mHeadMap;
	};
}

namespace Sentry
{
	class HttpHandlerResponse : public Tcp::ProtoMessage
	{
	 public:
		HttpHandlerResponse() = default;
	 public:
		bool Serailize(std::ostream &os) final;
		void Write(HttpStatus code, Json::Writer & content);
		void Write(HttpStatus code, const std::string & content);
		bool AddHead(const std::string & key, const std::string & value);
	private:
		HttpStatus mCode;
		std::string mContent;
		std::unordered_map<std::string, std::string> mHeadMap;
	};
}
#endif //GAMEKEEPER_HTTPASYNCREQUEST_H
